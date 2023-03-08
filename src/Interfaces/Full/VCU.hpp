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
        uint8_t required_torque_map_size = percent_division_count * speed_division_count;

        if (torque_map.size() != required_torque_map_size) {
            fmt::print("QML torque map size ({}) != required torque map size ({})\n", torque_map.size(), required_torque_map_size);
            return;
        }

        // Send header
        RetCode ans = CAN::Interface::write(0x0D0, std::array<uint8_t, 3>{
            'T', // Initiate upload transaction for (T)orque map
            percent_division_count, // Count of data points on percent axis
            speed_division_count, // Count of data points on speed axis
        });
        if (ans != RetCode::Success) {
            fmt::print("Failed to send header\n");
            return;
        }

        // Verify torque map is within bounds
        for (qsizetype i = 0; i < torque_map.size(); i++) {
            if (torque_map[i] > torque_map_max || torque_map[i] < torque_map_min) {
                fmt::print("Torque map values out of bounds\n");
                return;
            }
        }

        for (qsizetype i = 0; i < torque_map.size(); i++) {
            size_t packet_size = 0;
            std::array<uint8_t, 8> packet_values = {};
            for (; packet_size < 8 && i < torque_map.size(); packet_size++, i++) {
                int adjusted_data_point = torque_map[i] - torque_map_offset; // Readjust torque values to between -128 and 127
                int8_t data_point = static_cast<int8_t>(adjusted_data_point); // This is safe as values should already be within the bounds of this data type
                packet_values[packet_size] = *reinterpret_cast<uint8_t*>(&data_point); // Send as bitwise int8_t
            }

            RetCode ans = CAN::Interface::write(0x0D1, packet_values.data(), packet_size);
            if (ans != RetCode::Success) {
                fmt::print("Failed to send packet (size: {}, packet: {})\n", packet_size, packet_values);
                return;
            }
        }
    }

    void newFrame(const can_frame& frame) override {
        switch (CAN::frameId(frame)) {
            case 0x0D2:
                fmt::print("Torque map response (ack)\n");
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
