import asyncio
import cantools
import json
import os
import serial
import time
from foxglove_websocket import run_cancellable
from foxglove_websocket.server import FoxgloveServer, FoxgloveServerListener
from foxglove_websocket.types import (
    ChannelId,
    ClientChannel,
    ClientChannelId,
    ServiceId,
)

# serial port parameters
SERIAL_PORT = "/dev/ttyUSB0"
BAUD_RATE = 921600

# location of dbc folder (relative to this file)
DBC_FOLDER = "../DBCs"


def parse_can_message(message, db):
    if not message.strip():
        return (None, None)

    # split hex values
    hex_values = message.split(",")
    address = int(hex_values[0], 16)
    data = bytes([int(value, 16) for value in hex_values[1:]])

    # get message object and decoded data
    try:
        message = db.get_message_by_frame_id(address)
        decoded_message = db.decode_message(message.frame_id, data)
        return message.name, decoded_message

    except KeyError:
        return (None, None)


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

        ser = serial.Serial(SERIAL_PORT, BAUD_RATE)
        db = load_dbc_files(DBC_FOLDER)

        # create dictionary to store channel ids
        channel_ids = {}

        # generate channel id (and json schema) for each message
        for message in db.messages:
            topic_name = message.name
            schema = generate_json_schema(message)

            channel_id = await server.add_channel(
                {
                    "topic": topic_name,
                    "encoding": "json",
                    "schemaName": topic_name,
                    "schema": json.dumps(schema),
                    "schemaEncoding": "jsonschema",
                }
            )

            print(
                f"Added channel for topic {topic_name} with schema:\n{json.dumps(schema, indent=2)}"
            )

            # link channel id to associated message name
            channel_ids[topic_name] = channel_id

        # give time for foxglove to connect
        await asyncio.sleep(3)

        while True:
            # read and parse message from serial
            message = ser.readline().decode()
            message_name, decoded_message = parse_can_message(message, db)

            print(message_name)
            print(decoded_message)

            # check dictionary for message name
            if message_name in channel_ids:
                # temporary removal of this message (contains strings for certain data types)
                # TODO: generate correct json schemas for strings
                if message_name == "M170_Internal_States":
                    continue

                channel_id = channel_ids[message_name]

                # send message from server
                # TODO: change time variable to use RLM time
                await server.send_message(
                    channel_id,
                    time.time_ns(),
                    json.dumps(decoded_message).encode("utf8"),
                )


def generate_json_schema(message):
    schema = {"type": "object", "properties": {}}
    for signal in message.signals:
        schema["properties"][signal.name] = {"type": "number"}
    return schema


if __name__ == "__main__":
    run_cancellable(main())
