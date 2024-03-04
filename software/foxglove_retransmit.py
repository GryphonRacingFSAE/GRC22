# from hytech_np_proto_py import hytech_pb2
# import google.protobuf.message_factory
# from cantools.database import *
from base64 import b64decode
import asyncio
import websocket
from google.protobuf import descriptor_pb2
from google.protobuf import reflection
from google.protobuf import symbol_database
import protos.CAN_pb2


def gen(desc_file, db=None):
    if not db:
        db = symbol_database.Default()

    with open(desc_file, 'rb') as fh:
        fds = descriptor_pb2.FileDescriptorSet.FromString(fh.read())

    message_names = []
    message_classes = {}

    for prot in fds.file:
        fd = db.pool.Add(prot)
        for name in fd.message_types_by_name:
            message_names.append(name)
            message_classes[name] = google.protobuf.message_factory.GetMessageClass(fd.message_types_by_name[name]) 

    return message_names, message_classes



def on_message(wsapp, message):
    print(b64decode(message))

def pack_protobuf_msg(cantools_dict: dict, msg_name: str, message_classes):
    if msg_name in message_classes:
        pb_msg = message_classes[msg_name]()
    for key in cantools_dict.keys():
        if(type(cantools_dict[key]) is namedsignalvalue.NamedSignalValue):
            setattr(pb_msg, key, cantools_dict[key].value)
        else:
            setattr(pb_msg, key, cantools_dict[key])
    return pb_msg

    
if __name__ == "__main__":
    from pprint import pprint
    generated = gen("protos/protos.desc")
    pprint(generated)
    wsapp = websocket.WebSocketApp("ws://192.168.4.1:8765/ws", on_message=on_message)
    wsapp.run_forever()