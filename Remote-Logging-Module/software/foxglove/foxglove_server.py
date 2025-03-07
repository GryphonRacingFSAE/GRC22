import asyncio
import cantools
import datetime
import json
import os
import serial
import sys

from foxglove_websocket import run_cancellable
from foxglove_websocket.server import FoxgloveServer, FoxgloveServerListener
from foxglove_websocket.types import (
    ChannelId,
    ClientChannel,
    ClientChannelId,
    ServiceId,
)
from mcap.writer import Writer
from serial_asyncio import open_serial_connection

# check for command-line argument
if len(sys.argv) != 2:
    print("ERROR: No serial port selected")
    sys.exit()

# serial port parameters
SERIAL_PORT = sys.argv[1]
BAUD_RATE = 921600

# location of dbc folder (relative to this file)
DBC_FOLDER = "../../../DBCs"

# generate log folder if nonexistant
LOG_FOLDER = "./logs"
if not os.path.exists(LOG_FOLDER):
    os.makedirs(LOG_FOLDER)

# open new log file at current date+time
now = datetime.datetime.now()
log_file = open(LOG_FOLDER + now.strftime("/%Y-%m-%d_%H-%M-%S.mcap"), "wb")

# start log file writer (mcap format)
mcap_writer = Writer(log_file)
mcap_writer.start()


def parse_can_message(message, db):
    if not message.strip():
        return (None, None, None)

    # split messages by pipe
    parts = message.strip().split("|")
    if len(parts) != 3:
        return (None, None, None)

    addr_str, data_str, time_str = parts

    # extract address, data, and time
    try:
        address = int(addr_str)
        data = bytes([int(value, 16) for value in data_str.split(",")])
        time = int(time_str)
    except ValueError:
        return (None, None, None)

    # get message object and decoded data
    try:
        message = db.get_message_by_frame_id(address)
        decoded_message = db.decode_message(message.frame_id, data)
        return message.name, decoded_message, time
    except KeyError:
        return (None, None, None)


def load_dbc_files(folder):
    # create dbc database
    db = cantools.db.Database()

    # add all dbc files to database
    for file in os.listdir(folder):
        if file.endswith(".dbc"):
            dbc_file = os.path.join(folder, file)
            db.add_dbc_file(dbc_file)

    return db


async def main():
    # create server listener (for connect/disconnect events)
    class Listener(FoxgloveServerListener):
        async def on_subscribe(self, server: FoxgloveServer, channel_id: ChannelId):
            print("First client subscribed to", channel_id)

        async def on_unsubscribe(self, server: FoxgloveServer, channel_id: ChannelId):
            print("Last client unsubscribed from", channel_id)

    # create websocket server
    async with FoxgloveServer(
        "0.0.0.0",
        8765,
        "GRC22-RLM",
        capabilities=["clientPublish", "services"],
        supported_encodings=["json"],
    ) as server:
        server.set_listener(Listener())
        reader, writer = await open_serial_connection(
            url=SERIAL_PORT, baudrate=BAUD_RATE
        )
        db = load_dbc_files(DBC_FOLDER)

        # create dictionary to store channel ids
        fg_channel_ids = {}
        mcap_channel_ids = {}

        # generate channel id (and json schema) for each message
        for message in db.messages:
            topic_name = message.name
            schema = generate_json_schema(message)

            fg_channel_id = await server.add_channel(
                {
                    "topic": topic_name,
                    "encoding": "json",
                    "schemaName": topic_name,
                    "schema": json.dumps(schema),
                    "schemaEncoding": "jsonschema",
                }
            )

            print(f"Added channel for topic {topic_name}")

            # link channel id to associated message name
            fg_channel_ids[topic_name] = fg_channel_id

            # create associated mcap channel(s) for logging
            mcap_channel_id = add_mcap_channel(message, schema)
            mcap_channel_ids[topic_name] = mcap_channel_id

        while True:
            # read and parse message from serial
            message = await reader.readline()
            message_name, decoded_message, delta_time = parse_can_message(
                message.decode(), db
            )

            # print(message_name)
            # print(decoded_message)

            # check dictionary for message name
            if message_name in mcap_channel_ids:
                mcap_channel_id = mcap_channel_ids[message_name]
                fg_channel_id = fg_channel_ids[message_name]

                # write message to log file
                mcap_writer.add_message(
                    channel_id=mcap_channel_id,
                    log_time=(delta_time * 1000000),
                    data=json.dumps(decoded_message).encode("utf8"),
                    publish_time=(delta_time * 1000000),
                )

                # send message from server
                await server.send_message(
                    fg_channel_id,
                    delta_time,
                    json.dumps(decoded_message).encode("utf8"),
                )


def generate_json_schema(message):
    schema = {"type": "object", "properties": {}}
    for signal in message.signals:
        schema["properties"][signal.name] = {"type": "number"}
    return schema


def add_mcap_channel(message, schema):
    schema_id = mcap_writer.register_schema(
        name=message.name, encoding="jsonschema", data=json.dumps(schema).encode()
    )
    channel_id = mcap_writer.register_channel(
        schema_id=schema_id, topic=message.name, message_encoding="json"
    )
    return channel_id


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        mcap_writer.finish()  # close mcap file on exit
