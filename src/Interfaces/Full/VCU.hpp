#pragma once

#include <DBCInterface.hpp>
#include <QObject>
#include <limits>
#include <tools.hpp>
#include <fmt/ranges.h>

class VCU : public QObject, public CAN::Interface {
    Q_OBJECT
    Q_PROPERTY(int maxTorque MEMBER torque_map_max CONSTANT)
    Q_PROPERTY(int minTorque MEMBER torque_map_min CONSTANT)
  public:
    VCU(const std::string& torque_map_directory = "")
        : QObject(nullptr), m_torque_map_directory(torque_map_directory) {
            
        // Startup HW interface
        this->CAN::Interface::startReceiving(
            "can0", VCU::filters, VCU::num_of_filters, VCU::timeout_ms);
    }

    Q_INVOKABLE void sendTorqueMap(QList<int> torque_map){
        uint8_t percent_division_count = 11; // 0-100 in increments of 10%
        uint8_t speed_division_count = 14; // 0-130 in increments of 10kmph
        unsigned int expected_data_points_in_torque_map = percent_division_count * speed_division_count;
        if (expected_data_points_in_torque_map > 255) {
            fmt::print("Number of data points inside torque map ({}) exceeds limit of 255\n", expected_data_points_in_torque_map);
            return;
        }
        uint8_t num_of_torque_map_data_points = static_cast<uint8_t>(expected_data_points_in_torque_map);

        if (torque_map.size() != num_of_torque_map_data_points) {
            fmt::print("QML torque map size ({}) != required torque map size ({})\n", torque_map.size(), num_of_torque_map_data_points);
            return;
        }

        std::vector<uint8_t> transaction;
        transaction.reserve(num_of_torque_map_data_points);
        for (int torque : torque_map) {
            // Verify torque is within bounds
            if (torque > torque_map_max || torque < torque_map_min) {
                fmt::print("Torque map values out of bounds\n");
                return;
            }
            int adjusted_data_point = torque - torque_map_offset; // Readjust torque values to between -128 and 127
            int8_t data_point = static_cast<int8_t>(adjusted_data_point); // This is safe as values should already be within the bounds of this data type
            uint8_t encoded_data_point = *reinterpret_cast<uint8_t*>(&data_point); // Encode the int8_t into a uint8_t
            transaction.push_back(encoded_data_point);
        } 

        // Data for 2D arrays should be sent in Row Major Order
        auto header = std::array<uint8_t, 4>{
            'T', // Initiate upload transaction for (T)orque map
            num_of_torque_map_data_points, // Size of transaction in bytes
            // The last 6 bytes is reserved for data specific to the transaction
            speed_division_count, // Count of data points on speed axis (X axis or number of columns)
            percent_division_count, // Count of data points on percent axis (Y axis or number of rows)
        };

        if (sendTransaction(header, transaction) != RetCode::Success) {
            fmt::print("Something went wrong sending the transaction");
        }
    }

    template<size_t Header_N, class StorageClass>
    RetCode sendTransaction(std::array<uint8_t, Header_N> header, const StorageClass& transaction){
        static_assert(std::is_same<decltype(transaction.begin()), uint8_t>::value);
        // Send header
        RetCode ans = CAN::Interface::write(0x0D0, header);
        if (ans != RetCode::Success) {
            fmt::print("Failed to send header\n");
            return ans;
        }

        for (auto it = transaction.begin(); it != transaction.end(); it++) {
            size_t packet_size = 0;
            std::array<uint8_t, 8> packet_values = {};
            for (; packet_size < 8 && it != transaction.end(); packet_size++, it++) {
                packet_values[packet_size] = *it; 
            }

            RetCode ans = CAN::Interface::write(0x0D1, packet_values.data(), packet_size);
            if (ans != RetCode::Success) {
                fmt::print("Failed to send packet (size: {}, packet: {})\n", packet_size, packet_values);
                return ans;
            }
        }
        return RetCode::Success;
    }

    void newFrame(const can_frame& frame) override {
        switch (CAN::frameId(frame)) {
            case 0x0D2:
                fmt::print("({}) message ack, with data: \n", (char)frame.data[0], frame.data);
                break;
            default: 
                fmt::print("Anything else (lol)\n");
                break;
        }
    }
    void newError(const can_frame&) override {
        fmt::print("Error\n");
    } 
    void newTimeout() override{};

  private:
    std::string m_torque_map_directory;

  public:
    static constexpr int torque_map_offset = 128 - 25;
    static constexpr int torque_map_max = std::numeric_limits<int8_t>::max() + torque_map_offset; // Estimated is around -22Nm
    static constexpr int torque_map_min = std::numeric_limits<int8_t>::min() + torque_map_offset; // Estimated is around +230Nm
    static constexpr size_t num_of_filters = 1;
    inline static can_filter filters[num_of_filters] = {
        {
            0x0D0,
            0x0DF // Grab all messages VCU from 0D0 to 0DF (16 Addresses)
        }};

    static constexpr uint32_t timeout_ms = 500;
};
