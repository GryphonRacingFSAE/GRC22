#pragma once

#include <DBCInterface.hpp>
#include <QObject>
#include <filesystem>
#include <fmt/ranges.h>
#include <limits>
#include <rapidcsv.h>
#include <tools.hpp>

namespace real {

class VCU : public QObject, public CAN::DBCInterface<VCU> {
    Q_OBJECT
    Q_PROPERTY(QList<int> currentTorqueMap MEMBER m_current_torque_map NOTIFY currentTorqueMapChanged)
    Q_PROPERTY(int profileId MEMBER m_profile_id NOTIFY profileIdChanged)
    Q_PROPERTY(int maxTorque MEMBER torque_map_max CONSTANT)
    Q_PROPERTY(int minTorque MEMBER torque_map_min CONSTANT)
    Q_PROPERTY(QList<float> currentTcTune MEMBER m_current_tc_tune NOTIFY currentTcTuneChanged)
    Q_PROPERTY(int tcTuneId MEMBER m_tc_tune_id NOTIFY tcTuneIdChanged)
  public:
    VCU(const std::string& dbc_file = "VCU.dbc", const std::string& torque_map_directory = "")
        : QObject(nullptr), DBCInterface(dbc_file), m_torque_map_directory(torque_map_directory), m_profile_id(0), m_tc_tune_id(0) {

        // if (!std::filesystem::exists(m_torque_map_directory) || !std::filesystem::is_directory(m_torque_map_directory)) {
        //     fmt::print("Torque map directory doesn't exist!");
        //     throw "Bricked hard";
        // }

        // connect(this, &VCU::profileIdChanged, &VCU::readTorqueMapCSV);
        // connect(this, &VCU::tcTuneIdChanged, &VCU::readTcTuneCSV);

        // readTorqueMapCSV();
        // readTcTuneCSV();

        can_signal_dispatch["ACCELERATOR_POSITION"] = &VCU::newAcceleratorPos;
        can_signal_dispatch["BRAKE_PRESSURE"] = &VCU::newBrakePressure;        
        can_signal_dispatch["BSPC_INVALID"] = &VCU::newBSPCInvalid;
        can_signal_dispatch["APPS_OUT_OF_RANGE"] = &VCU::newAPPSOutOfRange;
        can_signal_dispatch["APPS_SENSOR_CONFLICT"] = &VCU::newAPPSSensorConflict;
        can_signal_dispatch["BRAKE_OUT_OF_RANGE"] = &VCU::newBrakeOutOfRange;
        can_signal_dispatch["CTRL_RTD_INVALID"] = &VCU::newRTDInvalid;
        can_signal_dispatch["RTD_BUTTON"] = &VCU::newRTDButton;
        can_signal_dispatch["BRAKE_SWITCH"] = &VCU::newBrakeSwitch;
        can_signal_dispatch["PUMP_ACTIVE"] = &VCU::newPumpActive;
        can_signal_dispatch["ACCUMULATOR_FAN_ACTIVE"] = &VCU::newAccumulatorFanActive;
        can_signal_dispatch["RADIATOR_FAN_ACTIVE"] = &VCU::newRadiatorFanActive;
    }

    Q_INVOKABLE void saveTorqueMapCSV(QList<int> torque_map) {
        auto save_path = m_torque_map_directory / fmt::format("torque_map_{}.csv", m_profile_id);
        rapidcsv::Document doc(save_path.string(), rapidcsv::LabelParams(-1, -1));
        for (qsizetype i = 0; i < torque_map.size(); i++) {
            doc.SetCell(i % 14, i / 14, torque_map.at(i));
        }
        doc.Save();
    }

    Q_INVOKABLE void saveTcTuneCSV(QList<float> tc_tune) {
        auto save_path = m_torque_map_directory / fmt::format("tc_tune_{}.csv", m_tc_tune_id);
        rapidcsv::Document doc(save_path.string(), rapidcsv::LabelParams(-1, -1));
        for (qsizetype i = 0; i < tc_tune.size(); i++) {
            doc.SetCell(i % 14, i / 14, tc_tune.at(i));
        }
        doc.Save();
    }

  public slots:
    void readTorqueMapCSV() {
        m_current_torque_map.clear();
        auto read_path = m_torque_map_directory / fmt::format("torque_map_{}.csv", m_profile_id);
        rapidcsv::Document doc(read_path.string(), rapidcsv::LabelParams(-1, -1));
        for (size_t i = 0; i < doc.GetRowCount(); i++) {
            for (const auto cell : doc.GetRow<int>(i)) {
                m_current_torque_map.push_back(cell);
            }
        }
        emit currentTorqueMapChanged();
    }

    void readTcTuneCSV() {
        m_current_tc_tune.clear();
        auto read_path = m_torque_map_directory / fmt::format("tc_tune_{}.csv", m_tc_tune_id);
        rapidcsv::Document doc(read_path.string(), rapidcsv::LabelParams(-1, -1));
        for (const auto cell : doc.GetRow<float>(0)) {
            m_current_tc_tune.push_back(cell);
        }

        emit currentTcTuneChanged();
    }

  signals:
    void currentTorqueMapChanged();
    void currentTcTuneChanged();
    void profileIdChanged();
    void tcTuneIdChanged();
    void newAcceleratorPos(float pos);
    void newBrakePressure(float psi);   
    void newBSPCInvalid(float state);
    void newAPPSOutOfRange(float state);
    void newAPPSSensorConflict(float state);
    void newBrakeOutOfRange(float state);
    void newRTDInvalid(float state);
    void newRTDButton(float state);
    void newBrakeSwitch(float state);
    void newPumpActive(float state);
    void newAccumulatorFanActive(float state);
    void newRadiatorFanActive(float state);

  public:
    Q_INVOKABLE void sendTorqueMap(QList<int> torque_map) {
        uint8_t percent_division_count = 11; // 0-100 in increments of 10%
        uint8_t speed_division_count = 14;   // 0-130 in increments of 10kmph
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
            int adjusted_data_point = torque - torque_map_offset;                  // Readjust torque values to between -128 and 127
            int8_t data_point = static_cast<int8_t>(adjusted_data_point);          // This is safe as values should already
                                                                                   // be within the bounds of this data type
            uint8_t encoded_data_point = *reinterpret_cast<uint8_t*>(&data_point); // Encode the int8_t into a uint8_t
            transaction.push_back(encoded_data_point);
        }

        // Headers must be 8 bytes long
        // Data for 2D arrays should be sent in Row Major Order
        auto header = std::array<uint8_t, 8>{
            0x00,                          // Dash is ID: 0
            'T',                           // Initiate upload transaction for (T)orque map
            num_of_torque_map_data_points, // Size of transaction in bytes
            0,                             // reserved
            // The last 4 bytes is reserved for data specific to the transaction
            speed_division_count,                                  // Count of data points on speed axis (X axis or number of
                                                                   // columns)
            percent_division_count,                                // Count of data points on percent axis (Y axis or number of
                                                                   // rows)
            *reinterpret_cast<const uint8_t*>(&torque_map_offset), // Send offset so VCU can decode the values
            0                                                      // Padding
        };

        if (sendTransaction(header, transaction) != RetCode::Success) {
            fmt::print("Something went wrong sending the transaction");
        }
    }

    // Q_INVOKABLE void sendTcTune(QList<float> tc_tune) {
    //     // TODO
    // }

    template <class StorageClass>
    RetCode sendTransaction(std::array<uint8_t, 8> header, const StorageClass& transaction) {
        static_assert(std::is_same<typename StorageClass::value_type, uint8_t>::value);
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

  private:
    std::filesystem::path m_torque_map_directory;
    QList<int> m_current_torque_map;
    QList<float> m_current_tc_tune;
    int m_profile_id;
    int m_tc_tune_id;

  public:
    static constexpr uint8_t torque_map_offset = 22;                  // -22 is target minimum
    static constexpr int torque_map_max =                             // Estimated is around -22Nm
        std::numeric_limits<uint8_t>::max() - (int)torque_map_offset; // Cast is required for integer promotion
    static constexpr int torque_map_min =                             // Estimated is around +230Nm
        std::numeric_limits<uint8_t>::min() - (int)torque_map_offset; // Cast is required for integer promotion
    static constexpr size_t num_of_filters = 1;
    inline static can_filter filters[num_of_filters] = {{
        0x200,
        0x7F0 // Grab all messages VCU from 0x200 to 0x20F (16 Addresses)
    }};

    static constexpr uint32_t timeout_ms = 500;
};

} // namespace real