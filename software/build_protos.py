Import("env")

from DBCs.dbc_to_protobuf import DBCToProto


print("Current CLI targets", COMMAND_LINE_TARGETS)
print("Current Build targets", BUILD_TARGETS)

# def post_program_action(source, target, env):
#     print("Program has been built!")
#     program_path = target[0].get_abspath()
#     print("Program path", program_path)
#     # Use case: sign a firmware, do any manipulations with ELF, etc
#     # env.Execute(f"sign --elf {program_path}")

# env.AddPostAction("$PROGPATH", post_program_action)

#
# Upload actions
#

def before_build(source, target, env):
    # Generate protobuf files from DBCs
    dbc_file_paths = [os.path.join(self.recipe_folder, "DBCs", file) for file in os.listdir(os.path.join(self.recipe_folder, "DBCs")) if file.endswith(".dbc")]
    DBCToProto(dbc_file_paths, os.path.join(self.recipe_folder, "protos"))


    # # Generate descriptor set
    # env.Execute("protoc --include_imports --descriptor_set_out=${CMAKE_BINARY_DIR}/bin/protos.desc --proto_path=${CMAKE_SOURCE_DIR}/protos ${CMAKE_SOURCE_DIR}/protos/*.proto")


def after_upload(source, target, env):
    print("after_upload")
    # do some actions

env.AddPreAction("build", before_build)
env.AddPostAction("upload", after_upload)