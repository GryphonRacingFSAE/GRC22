#pragma once

#if defined(__linux__)
#include <linux/can.h>
#include <unistd.h>
#else
#error "Cannot build backend if not on linux"
#endif

#include <dbcppp/Network.h>
#include <foxglove/websocket/server_interface.hpp>
#include <google/protobuf/descriptor.pb.h>
#include <mcap/writer.hpp>
#include <websocketpp/common/connection_hdl.hpp>

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

class FGLogger {
  public:
    FGLogger(std::string dbc_folder, std::string protobuf_desc_file, uint16_t publishing_port);
    virtual ~FGLogger();

    void saveAndPublish(const can_frame& can_frame);

  private:
    std::vector<std::unique_ptr<dbcppp::INetwork>> dbc_networks;
    google::protobuf::DescriptorPool descriptor_pool;
    mcap::McapWriter mcap_writer;
    std::unordered_map<std::string, foxglove::ChannelId> websocket_message_to_channel_id_map;
    std::unordered_map<std::string, mcap::ChannelId> mcap_message_to_channel_id_map;
    std::unordered_map<std::string, const google::protobuf::Descriptor*> message_to_message_descriptor_map;
    std::unique_ptr<foxglove::ServerInterface<websocketpp::connection_hdl>> server;
    std::unordered_map<uint64_t, const dbcppp::IMessage*> can_message_mapping;

    static void log(foxglove::WebSocketLogLevel, char const* msg);
    std::pair<std::string, std::string> serializeCANToProtobuf(const can_frame& can_frame);
};