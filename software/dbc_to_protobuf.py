# Adapted from https://github.com/RCMast3r/data_acq (Hytech Racing)

import cantools
from cantools.database import *
import os
import pathlib

def CANMessageToProtoMessageText(can_msg: can.message.Message):
    # type and then name
    proto_message = f"message {can_msg.name} {{\n"
    for (line_index, sig) in enumerate(can_msg.signals, start=1):
        
        # if the msg has a conversion, we know that the value with be a float    
        if sig.is_float or ((sig.scale is not None) and (sig.scale != 1.0)):
            proto_message += f"\tfloat {sig.name} = {line_index};\n"
        elif sig.length == 1:
            proto_message += f"\tbool {sig.name} = {line_index};\n"
        elif sig.length > 32 and sig.is_signed:
            proto_message += f"\tint64 {sig.name} = {line_index};\n" # We can decide later if we want to use sint instead of int.
        elif sig.length > 1 and sig.is_signed:
            proto_message += f"\tint32 {sig.name} = {line_index};\n"
        elif sig.length > 32:
            proto_message += f"\tuint64 {sig.name} = {line_index};\n"
        elif sig.length > 1:
            proto_message += f"\tuint32 {sig.name} = {line_index};\n"
        else:
            print("Missing representation in protobuf from DBC!")
            return None
    proto_message += f"}}\n\n"
    return proto_message

def DBCToProto(paths: [str], result_dir: str):
    for path in paths:
        can_db = cantools.database.load_file(path)
        with open(f"{result_dir}/{pathlib.Path(path).stem}.proto", "w+") as proto_file:
            proto_file.write('syntax = "proto3";\n\n')
            for msg in can_db.messages:
                proto_file.write(CANMessageToProtoMessageText(msg))

if __name__ == "__main__":
    DBCToProto(["./src/Interfaces/DBCs/SMU.dbc"], ".")