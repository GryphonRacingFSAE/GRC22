set(CODE_FILES BusDump.cpp Dump.cpp)

set(HEADER_FILES Dump.hpp)

add_executable(BusDump ${CODE_FILES} ${HEADER_FILES})
set_target_properties(BusDump PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<0:>)
target_link_libraries(
    BusDump
    PRIVATE FGLoggerLib
            dbcppp::dbcppp
            protobuf::protobuf
            mcap::mcap
            foxglove-websocket::foxglove-websocket
            fmt::fmt
            Threads::Threads
            CANLib)
target_include_directories(BusDump PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(
    TARGET BusDump
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/DBCs $<TARGET_FILE_DIR:BusDump>/DBCs)

add_custom_command(
    TARGET BusDump
    PRE_BUILD
    COMMAND ${Protobuf_PROTOC_EXECUTABLE} --include_imports --descriptor_set_out=$<TARGET_FILE_DIR:BusDump>/protos.desc
            --proto_path=${CMAKE_SOURCE_DIR}/protos ${CMAKE_SOURCE_DIR}/protos/*.proto)
