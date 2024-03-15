from SCons.Script import *

from dbc_to_protobuf import DBCToProto

import os

# Generate protobuf files from DBCs
dbc_file_paths = [os.path.join(os.getcwd(), "../DBCs", file) for file in os.listdir(os.path.join(os.getcwd(), "../DBCs")) if file.endswith(".dbc")]
DBCToProto(dbc_file_paths, os.path.join(os.getcwd(), "protos"))

# Generate descriptor set
env = Environment()
env.Execute(f"protoc --include_imports --descriptor_set_out={os.path.join(os.getcwd(), 'protos', 'protos.desc')} --proto_path={os.path.join(os.getcwd(), 'protos')} {os.path.join(os.getcwd(), 'protos', '*.proto')}")
env.Execute(f"protoc --python_out={os.path.join(os.getcwd(), 'protos')} --proto_path={os.path.join(os.getcwd(), 'protos')} {os.path.join(os.getcwd(), 'protos', 'CAN.proto')}")
