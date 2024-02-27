#define MCAP_IMPLEMENTATION
#include <Dump.hpp>
#include <tools.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>

#include <fmt/core.h>

#include <chrono>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
namespace pb = google::protobuf;
using namespace CAN::Interfaces;

Dump::~Dump() {
    this->Interface::stopReceiving();
}

Dump::Dump(std::string dbc_folder) {
    if (!fs::exists(dbc_folder) || !fs::is_directory(dbc_folder)) {
        fmt::print("DBC folder does not exist or isn't a folder!\n");
    }

    for (auto const& dir_entry : fs::directory_iterator{dbc_folder}) {
        if (!fs::is_directory(dir_entry.path()) && dir_entry.path().extension() == ".dbc") {
            fmt::print("Loading DBC from: {}\n", dir_entry.path().filename().string());
            std::ifstream dbc_file(dir_entry.path().string());
            dbc_networks.emplace_back(dbcppp::INetwork::LoadDBCFromIs(dbc_file));
        }
    }

    auto options = mcap::McapWriterOptions("");
    const auto res = mcap_writer.open("test.mcap", options);
    if (!res.ok()) {
        fmt::print("Failed to open for writing!\n");
        return;
    }

    google::protobuf::FileDescriptorSet proto_fd_set;
    std::ifstream proto_desc_file("protos.desc", std::ios::binary);
    if (!proto_fd_set.ParseFromIstream(&proto_desc_file)) {
        fmt::print("Wasn't able to decode file!\n");
        return;
    }
    proto_desc_file.close();

    for (int i = 0; i < proto_fd_set.file_size(); i++) {
        fmt::print("Using data from: {}\n", proto_fd_set.file(i).name());
        if (descriptor_pool.BuildFile(proto_fd_set.file(i)) == nullptr) {
            fmt::print("Was unable to load {} as a file descriptor!\n", proto_fd_set.file(i).name());
        }
    }

    for (const std::unique_ptr<dbcppp::INetwork>& network : this->dbc_networks) {
        for (const dbcppp::IMessage& msg : network->Messages()) {
            // Make a channel on the writer for each message in each DBC
            const pb::Descriptor* message_descriptor = nullptr;

            for (int i = 0; i < proto_fd_set.file_size() && message_descriptor == nullptr; i++) {
                message_descriptor = descriptor_pool.FindMessageTypeByName(msg.Name());
            }

            if (message_descriptor != nullptr) {
                fmt::print("Adding schema and channel for: {}\n", msg.Name());

                mcap::Schema schema(msg.Name(), "protobuf", proto_fd_set.SerializeAsString());
                mcap_writer.addSchema(schema);

                mcap::Channel channel(msg.Name(), "protobuf", schema.id);
                mcap_writer.addChannel(channel);
                message_to_channel_id_map[msg.Name()] = channel.id;
                message_to_message_descriptor_map[msg.Name()] = message_descriptor;
            }
        }
    }
}

void Dump::startReceiving() {
    this->Interface::startReceiving("can0", nullptr, 0, Dump::timeout_ms); // Get everything
}

void Dump::newFrame(const can_frame& frame) {
    // Grab the timestamp for the mcap with the lowest latency we can.
    mcap::Timestamp frame_timestamp =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    fmt::print("ID: 0x{:08X}, Ext: {}, RTR: {}, Err: {}, Payload: 0x{:02X} 0x{:02X} 0x{:02X} "
               "0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X}\n",
               CAN::frameId(frame),
               CAN::frameFormat(frame) == CanFormat::Extended,
               CAN::isRemoteTransmissionRequest(frame),
               CAN::isError(frame),
               frame.data[0],
               frame.data[1],
               frame.data[2],
               frame.data[3],
               frame.data[4],
               frame.data[5],
               frame.data[6],
               frame.data[7]);

    // Take the can frame, convert it to it's protobuf analog, and then save it to mcap
    for (const std::unique_ptr<dbcppp::INetwork>& network : this->dbc_networks) {
        for (const dbcppp::IMessage& msg : network->Messages()) {
            if (msg.Id() == frame.can_id) {
                fmt::print("Found matching network & message ({}) for ID: {}\n", msg.Name(), frame.can_id);

                pb::DynamicMessageFactory dmf;

                // Create a protobuf message utilizing the
                const pb::Descriptor* message_descriptor = message_to_message_descriptor_map[msg.Name()];
                pb::Message* actual_msg = dmf.GetPrototype(message_descriptor)->New(); // Check for nullptr?

                const pb::Reflection* refl = actual_msg->GetReflection();
                for (const dbcppp::ISignal& sig : msg.Signals()) {
                    const pb::FieldDescriptor* fd = message_descriptor->FindFieldByName(sig.Name());
                    if (fd == nullptr) {
                        fmt::print("Missing field in protobuf message descriptor! Message: {} Signal: {}\n", msg.Name(), sig.Name());
                        return;
                    }

                    // Ensure we set the field with the correct type
                    switch (fd->type()) {
                    case pb::FieldDescriptor::TYPE_INT32:
                        refl->SetInt32(actual_msg, fd, sig.RawToPhys(sig.Decode(frame.data)));
                        break;
                    case pb::FieldDescriptor::TYPE_INT64:
                            // This might cut off data due to limits in the size of doubles
                        refl->SetInt64(
                            actual_msg, fd, sig.RawToPhys(sig.Decode(frame.data))); 
                        break;
                    case pb::FieldDescriptor::TYPE_UINT32:
                        refl->SetUInt32(actual_msg, fd, sig.RawToPhys(sig.Decode(frame.data)));
                        break;
                    case pb::FieldDescriptor::TYPE_UINT64:
                            // This might cut off data due to limits in the size of doubles
                        refl->SetUInt64(
                            actual_msg, fd, sig.RawToPhys(sig.Decode(frame.data))); 
                        break;
                    case pb::FieldDescriptor::TYPE_FLOAT:
                        refl->SetFloat(actual_msg, fd, sig.RawToPhys(sig.Decode(frame.data)));
                        break;
                    case pb::FieldDescriptor::TYPE_DOUBLE:
                        refl->SetDouble(actual_msg, fd, sig.RawToPhys(sig.Decode(frame.data)));
                        break;
                    default:
                        fmt::print("Missing type handling for field descriptor type: {}\n", fd->type());
                        return;
                    }
                }

                // Taken from https://github.com/foxglove/mcap/blob/main/cpp/examples/protobuf/writer.cpp
                std::string serialized = actual_msg->SerializeAsString();

                mcap::Message mcap_msg;
                mcap_msg.channelId = message_to_channel_id_map[msg.Name()];
                mcap_msg.sequence = 0; // We can assign a counter to this to measure gaps between messages
                mcap_msg.publishTime = frame_timestamp;
                mcap_msg.logTime = frame_timestamp;
                mcap_msg.data = reinterpret_cast<const std::byte*>(serialized.data());
                mcap_msg.dataSize = serialized.size();
                const auto res = mcap_writer.write(mcap_msg);

                if (!res.ok()) {
                    fmt::print("Failed to write message: {}\n", res.message);
                    mcap_writer.terminate();
                    mcap_writer.close();
                }

                return;
            }
        }
    }
}

void Dump::newError(const can_frame&) {
    fmt::print("Error\n");
}

void Dump::newTimeout() {
    fmt::print("Timeout\n");
}