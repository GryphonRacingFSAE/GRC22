#pragma once

#include <Interface.hpp>
#include <dbcppp/Network.h>
#include <google/protobuf/descriptor.pb.h>
#include <mcap/writer.hpp>
#include <string>
#include <vector>

namespace CAN::Interfaces {

class Dump : public Interface {
  public:
    Dump(std::string dbc_folder = "DBCs");
    virtual ~Dump() = default;
    void startReceiving();

  private:
    // As of now, newFrame and newError should be extremely fast functions before emitting more
    // signals
    void newFrame(const can_frame&) override;
    void newError(const can_frame&) override;
    void newTimeout() override;

  private:
    static constexpr uint32_t timeout_ms = 500;
    std::vector<std::unique_ptr<dbcppp::INetwork>> dbc_networks;
    // google::protobuf::DescriptorPool descriptor_pool;
    // mcap::McapWriter mcap_writer;
    // std::unordered_map<std::string, mcap::ChannelId> message_to_channel_id_map;
    // std::unordered_map<std::string, const google::protobuf::Descriptor*> message_to_message_descriptor_map;
};

} // namespace CAN::Interfaces