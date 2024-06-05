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
    Q_PROPERTY(int maxPower MEMBER m_max_power)
    Q_PROPERTY(int maxTorque MEMBER m_max_torque)
  public:
    VCU(const std::string& dbc_file = "VCU.dbc", const std::string& torque_map_directory = "")
        : QObject(nullptr), DBCInterface(dbc_file){

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
        sendTorqueConfig();
    }

    Q_INVOKABLE void sendTorqueConfig() {
        // std::array<uint8_t, 6> packet_values = {};
        // packet_values[0] = ;
        // packet_values[1] = m_max_torque & ;
        // packet_values[2] = *it;
        // packet_values[3] = *it;
        // packet_values[4] = *it;
        // packet_values[5] = *it;

        // RetCode ans = CAN::Interface::write(0x300, packet_values.data(), 6);
        // if (ans != RetCode::Success) {
        //     fmt::print("Failed to send packet (size: {}, packet: {})\n", packet_size, packet_values);
        //     return ans;
        // }
    }


  signals:
    void currentTorqueMapChanged();
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
  private:
    int m_max_torque;
    int m_max_power;

  public:
    static constexpr size_t num_of_filters = 1;
    inline static can_filter filters[num_of_filters] = {{
        0x300,
        0x7F0 // Grab all messages VCU from 0x200 to 0x20F (16 Addresses)
    }};

    static constexpr uint32_t timeout_ms = 500;
};

} // namespace real