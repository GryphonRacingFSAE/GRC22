# from hytech_np_proto_py import hytech_pb2
# from cantools.database import *
from base64 import b64decode, b64encode
import asyncio
import websockets
from google.protobuf import descriptor_pb2
from google.protobuf import reflection
from google.protobuf import symbol_database
from google.protobuf import message_factory
from foxglove_websocket import run_cancellable
from protos import CAN_2_pb2
import can
import cantools
import time
import glob


from foxglove_websocket.server import FoxgloveServer, FoxgloveServerListener
from foxglove_websocket.types import ChannelId


async def gen(desc_file, fg_server):
    with open(desc_file, 'rb') as fh:
        file_descriptor_set = descriptor_pb2.FileDescriptorSet.FromString(fh.read())

    schema = b64encode(file_descriptor_set.SerializeToString()).decode("ascii")

    message_descriptor_dict = message_factory.GetMessages(file_descriptor_set.file)

    channel_id_mapping = {}

    for name, messages in message_descriptor_dict.items():
        channel_id_mapping[name] = await fg_server.add_channel({
            "topic": name,
            "encoding": "protobuf",
            "schemaName": name,
            "schema": schema,
            "schemaEncoding": "protobuf"
        })

    return channel_id_mapping, message_descriptor_dict


def pack_protobuf_msg(cantools_dict: dict, msg_name: str, message_classes):
    if msg_name in message_classes:
        pb_msg = message_classes[msg_name]()
    else:
        print("message is not in protobuf messages")
        return

    for key in cantools_dict.keys():
        if(type(cantools_dict[key]) is cantools.database.namedsignalvalue.NamedSignalValue):
            setattr(pb_msg, key, cantools_dict[key].value)
        else:
            setattr(pb_msg, key, cantools_dict[key])
    return pb_msg

    

def gendbc():
    dbfiles = glob.glob("DBCs/*.dbc")

    db = cantools.database.Database()

    for file in dbfiles:
        db.add_dbc_file(file)

    return db

async def main():
    class Listener(FoxgloveServerListener):
        async def on_subscribe(self, server: FoxgloveServer, channel_id: ChannelId):
            print("First client subscribed to", channel_id)

        async def on_unsubscribe(self, server: FoxgloveServer, channel_id: ChannelId):
            print("Last client unsubscribed from", channel_id)

    async with FoxgloveServer("0.0.0.0", 8765, "Gryphon Racing Base Station") as fg_server, websockets.connect("ws://192.168.4.1:8765/ws") as websocket:
        fg_server.set_listener(Listener())
        channel_id_mapping, message_descriptor_dict_pb = await gen("protos/protos.desc", fg_server)
        message_dbc = gendbc()

        # client connection handlerasync with t:
        async for message in websocket:
            can_protobuf_msg = CAN_2_pb2.CAN()
            can_protobuf_msg.ParseFromString(b64decode(message))
            msg = can.Message(arbitration_id=can_protobuf_msg.address, dlc=len(can_protobuf_msg.data), data=can_protobuf_msg.data)
            print(msg)
            try:
                decoded_msg = message_dbc.decode_message(msg.arbitration_id, msg.data, decode_containers=True)
                dbc_msg = message_dbc.get_message_by_frame_id(msg.arbitration_id)
                pb_msg = pack_protobuf_msg(decoded_msg, dbc_msg.name, message_descriptor_dict_pb)
                asyncio.create_task(fg_server.send_message(channel_id_mapping[dbc_msg.name], time.time_ns(), pb_msg.SerializeToString()))
            except Exception as e:
                print("unable to decode...", e)
                return

if __name__ == "__main__":
    run_cancellable(main())