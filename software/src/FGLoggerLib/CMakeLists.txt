set(CODE_FILES FGLogger.cpp)

set(HEADER_FILES FGLogger.hpp)

add_library(FGLoggerLib ${CODE_FILES} ${HEADER_FILES})
set_target_properties(FGLoggerLib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/$<0:>)
target_link_libraries(
    FGLoggerLib
    PUBLIC  mcap::mcap
            foxglove-websocket::foxglove-websocket
    PRIVATE dbcppp::dbcppp
            protobuf::protobuf
            fmt::fmt
            Threads::Threads)
target_include_directories(FGLoggerLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(
    TARGET FGLoggerLib
    PRE_BUILD
    COMMAND ${Protobuf_PROTOC_EXECUTABLE} --include_imports --descriptor_set_out=${CMAKE_BINARY_DIR}/bin/protos.desc
            --proto_path=${CMAKE_SOURCE_DIR}/protos ${CMAKE_SOURCE_DIR}/protos/*.proto)
