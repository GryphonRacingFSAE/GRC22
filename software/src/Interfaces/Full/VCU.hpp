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
  public:
    VCU(const std::string& dbc_file = "VCU.dbc") : QObject(nullptr), DBCInterface(dbc_file) {

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
        can_signal_dispatch["STORED_TORQUE"] = &VCU::newStoredTorque;
        can_signal_dispatch["STORED_POWER"] = &VCU::newStoredPower;
        can_signal_dispatch["STORED_SPEED_LIMIT"] = &VCU::newStoredSpeedLimit;
        can_signal_dispatch["STORED_IDLE_PUMP_SPEED"] = &VCU::newStoredIdlePumpSpeed;
    }

    Q_INVOKABLE void sendTorqueConfig(uint16_t torque, uint16_t power, uint16_t rpm, uint8_t pump) {
        std::array<uint8_t, 7> packet_values = {};
        packet_values[0] = (torque * 10) & 0xFF;
        packet_values[1] = ((torque * 10) >> 8) & 0xFF;
        packet_values[2] = (power * 10) & 0xFF;
        packet_values[3] = ((power * 10) >> 8) & 0xFF;
        packet_values[4] = rpm & 0xFF;
        packet_values[5] = (rpm >> 8) & 0xFF;
        packet_values[6] = pump & 0xFF;

        RetCode ans = CAN::Interface::write(0x400, packet_values.data(), 7);
        if (ans != RetCode::Success) {
            fmt::print("Failed to send packet (size: {}, packet: {})\n", 7, packet_values);
        } else {
            fmt::print("Sent Config: Torque: {}, Power: {}, RPM: {}, Idle Speed: {}\n", torque, power, rpm, pump);
        }
    }

  signals:
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
    void newStoredTorque(float);
    void newStoredPower(float);
    void newStoredSpeedLimit(float);
    void newStoredIdlePumpSpeed(float);

  public:
    static constexpr size_t num_of_filters = 1;
    inline static can_filter filters[num_of_filters] = {{
        0x300,
        0x7F0 // Grab all messages VCU from 0x300 to 0x30F (16 Addresses)
    }};

    static constexpr uint32_t timeout_ms = 500;
};

} // namespace real