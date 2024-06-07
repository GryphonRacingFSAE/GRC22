#define MCAP_IMPLEMENTATION
#include <FGLogger.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>

#include <foxglove/websocket/base64.hpp>
#include <foxglove/websocket/server_factory.hpp>
#include <foxglove/websocket/websocket_notls.hpp>
#include <foxglove/websocket/websocket_server.hpp>

#include <fmt/chrono.h>
#include <fmt/core.h>

#include <chrono>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
namespace pb = google::protobuf;

FGLogger::~FGLogger() {
    server->stop();
}

void FGLogger::log(foxglove::WebSocketLogLevel, char const* msg) {
    fmt::print("Foxglove: {}\n", msg);
}

void FGLogger::restartSaving() {
    if (!fs::exists(saving_folder_path) || !fs::is_directory(saving_folder_path)) {
        fmt::print("Saving path does not exist or isn't a folder!\n");
        return;
    }

    auto now = std::chrono::system_clock::now();
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    std::string folder_path = fmt::format("{}/{:%Y-%m-%d}", saving_folder_path, now);
    if (!fs::exists(folder_path)) {
        fs::create_directory(folder_path);
    }

    auto options = mcap::McapWriterOptions("");
    std::string save_path = fmt::format("{}/{}.mcap", folder_path, timestamp);
    fmt::print("Saving to: {}\n", save_path);
    const auto res = mcap_writer.open(save_path, options);
    if (!res.ok()) {
        fmt::print("Failed to open for writing!\n");
        return;
    }
}

FGLogger::FGLogger(std::string dbc_folder, std::string protobuf_desc_file, uint16_t publishing_port, std::string saving_folder_path)
    : saving_folder_path(saving_folder_path) {
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

    this->restartSaving();

    foxglove::ServerOptions serverOptions;
    server =
        foxglove::ServerFactory::createServer<websocketpp::connection_hdl>("Gryphon Racing Foxglove Protobuf Server", FGLogger::log, serverOptions);

    foxglove::ServerHandlers<foxglove::ConnHandle> hdlrs;
    hdlrs.subscribeHandler = [](foxglove::ChannelId channel_id, foxglove::ConnHandle) { fmt::print("First client subscribed to: {}\n", channel_id); };
    hdlrs.unsubscribeHandler = [](foxglove::ChannelId channel_id, foxglove::ConnHandle) {
        fmt::print("Last client subscribed to: {}\n", channel_id);
    };
    server->setHandlers(std::move(hdlrs));
    server->start("0.0.0.0", publishing_port);

    google::protobuf::FileDescriptorSet proto_fd_set;
    std::ifstream proto_desc_file(protobuf_desc_file, std::ios::binary);
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

            // We only care about our CAN messages if they overlap with our protobuf,
            // which they always should because the protobuf files are generated from the DBCs
            if (message_descriptor == nullptr) {
                fmt::print("Missing protobuf descriptor for: {}!\n", msg.Name());
                continue;
            }

            // Insert message mapping for easier lookup
            can_message_mapping.insert(std::make_pair(msg.Id(), &msg));

            fmt::print("Adding schema and channel for: {}\n", msg.Name());

            mcap::Schema schema(msg.Name(), "protobuf", proto_fd_set.SerializeAsString());
            mcap_writer.addSchema(schema);

            mcap::Channel channel(msg.Name(), "protobuf", schema.id);
            mcap_writer.addChannel(channel);
            mcap_message_to_channel_id_map[msg.Name()] = channel.id;

            const auto channel_ids = server->addChannels({{
                .topic = msg.Name(),
                .encoding = "protobuf",
                .schemaName = msg.Name(),
                .schema = foxglove::base64Encode(proto_fd_set.SerializeAsString()),
                .schemaEncoding = "protobuf",
            }});
            websocket_message_to_channel_id_map[msg.Name()] = channel_ids.front();

            message_to_message_descriptor_map[msg.Name()] = message_descriptor;
        }
    }
}

std::pair<std::string, std::string> FGLogger::serializeCANToProtobuf(const can_frame& can_frame) {
    // Get the CAN message decoder
    const auto& message = can_message_mapping.find(can_frame.can_id);
    if (message == can_message_mapping.end()) {
        return std::make_pair("", ""); // Could not find decoding logic for message in the provided DBCs
    }
    const dbcppp::IMessage& msg = *message->second;
    // fmt::print("Found matching network & message ({}) for ID: {}\r", msg.Name(), can_frame.can_id);

    // Create a protobuf message matching the DBC CAN message
    pb::DynamicMessageFactory dmf;
    const pb::Descriptor* message_descriptor = message_to_message_descriptor_map[msg.Name()];
    pb::Message* actual_msg = dmf.GetPrototype(message_descriptor)->New(); // Check for nullptr?

    // Populate the protobuf messages with values located in the CAN message (while decoding using the DBCs)
    const pb::Reflection* refl = actual_msg->GetReflection();
    for (const dbcppp::ISignal& sig : msg.Signals()) {
        const pb::FieldDescriptor* fd = message_descriptor->FindFieldByName(sig.Name());
        if (fd == nullptr) {
            fmt::print("Missing field in protobuf message descriptor! Message: {} Signal: {}\n", msg.Name(), sig.Name());
            return std::make_pair("", "");
        }

        // Ensure we set the field with the correct type
        switch (fd->type()) {
        case pb::FieldDescriptor::TYPE_BOOL:
            refl->SetBool(actual_msg, fd, sig.Decode(can_frame.data));
            break;
        case pb::FieldDescriptor::TYPE_INT32: {
            uint64_t decoded = sig.Decode(can_frame.data);
            int32_t value = *reinterpret_cast<int32_t*>(&decoded) * sig.Factor() + sig.Offset();
            refl->SetInt32(actual_msg, fd, value);
            break;
        }
        case pb::FieldDescriptor::TYPE_INT64: {
            uint64_t decoded = sig.Decode(can_frame.data);
            int64_t value = *reinterpret_cast<int64_t*>(&decoded) * sig.Factor() + sig.Offset();
            refl->SetInt64(actual_msg, fd, value);
            break;
        }
        case pb::FieldDescriptor::TYPE_UINT32: {
            uint64_t decoded = sig.Decode(can_frame.data);
            uint32_t value = *reinterpret_cast<uint32_t*>(&decoded) * sig.Factor() + sig.Offset();
            refl->SetUInt32(actual_msg, fd, value);
            break;
        }
        case pb::FieldDescriptor::TYPE_UINT64: {
            uint64_t decoded = sig.Decode(can_frame.data);
            uint64_t value = decoded * sig.Factor() + sig.Offset();
            refl->SetUInt64(actual_msg, fd, value);
            break;
        }
        case pb::FieldDescriptor::TYPE_FLOAT:
            refl->SetFloat(actual_msg, fd, sig.RawToPhys(sig.Decode(can_frame.data)));
            break;
        case pb::FieldDescriptor::TYPE_DOUBLE:
            refl->SetDouble(actual_msg, fd, sig.RawToPhys(sig.Decode(can_frame.data)));
            break;
        default:
            fmt::print("Missing type handling for field: {}\n", sig.Name());
            return std::make_pair("", "");
        }
    }

    // Taken from https://github.com/foxglove/mcap/blob/main/cpp/examples/protobuf/writer.cpp
    return std::make_pair(msg.Name(), actual_msg->SerializeAsString());
}

void FGLogger::saveAndPublish(const can_frame& can_frame, const uint64_t timestamp) {
    // Grab the timestamp for the mcap with the lowest latency we can.
    mcap::Timestamp frame_timestamp =
        timestamp ? timestamp : std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    const auto& [message_name, serialized_message] = FGLogger::serializeCANToProtobuf(can_frame);
    if (serialized_message.empty()) {
        return;
    }

    // Hopefully this fixes it but if the USB is removed it won't
    if (mcap_writer.dataSink() == nullptr) {
        this->restartSaving();
    }

    // Save the protobuf message to a local mcap file.
    mcap::Message mcap_msg;
    mcap_msg.channelId = mcap_message_to_channel_id_map[message_name];
    mcap_msg.sequence = 0; // We can assign a counter to this to measure gaps between messages
    mcap_msg.publishTime = frame_timestamp;
    mcap_msg.logTime = frame_timestamp;
    mcap_msg.data = reinterpret_cast<const std::byte*>(serialized_message.data());
    mcap_msg.dataSize = serialized_message.size();
    const auto res = mcap_writer.write(mcap_msg);

    if (!res.ok()) {
        fmt::print("Failed to write message: {}\n", res.message);
        mcap_writer.terminate();
        mcap_writer.close();
    }

    // Send the message out via foxglove-websocket
    server->broadcastMessage(websocket_message_to_channel_id_map[message_name],
                             frame_timestamp,
                             reinterpret_cast<const uint8_t*>(serialized_message.data()),
                             serialized_message.size());
}
