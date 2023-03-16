#pragma once

#include <DBCInterface.hpp>
#include <Frontend/FakeInterface.hpp>
#include <QObject>

namespace demo {

class MotorController : public QObject, public CAN::DBCInterface<MotorController>, CAN::FakeInterface {
    Q_OBJECT
  public:
    MotorController(const std::string& dbc_file_path = "20220510_Gen5_CAN_DB.dbc")
        : QObject(nullptr), DBCInterface(dbc_file_path) {
        can_signal_dispatch["INV_Motor_Speed"] = &MotorController::newMotorRPM;
        can_signal_dispatch["INV_Motor_Temp"] = &MotorController::newMotorTemp;
        can_signal_dispatch["INV_Coolant_Temp"] = &MotorController::newCoolantTemp;
        can_signal_dispatch["INV_Analog_Input_1"] = &MotorController::new12VVoltage;
        can_signal_dispatch["INV_Analog_Input_2"] = &MotorController::newOilTemp;
        can_signal_dispatch["INV_Module_A_Temp"] = &MotorController::newModuleATemp;
        can_signal_dispatch["VCU_INV_Torque_Command"] = &MotorController::newRequestedTorqueHandler;
        // can_signal_dispatch["INV_Torque_Feedback"] = &MotorController::newOutputTorqueHandler;
    }

    Q_INVOKABLE void clearFaultCodes() {
        // CM200DZ CAN Protocol V5.9 Section 2.3.1 and 2.3.3 (Address 20 or 0x14)
        RetCode ans = CAN::Interface::write(
            0x0C1, std::array<uint8_t, 8>{0x00, 0x14, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00});

        if (ans != RetCode::Success) {
            fmt::print("Failed to clear fault codes\n");
        }
    }

  signals:
    void newMotorRPM(float rpm);
    void newMotorTemp(float temp);
    void newCoolantTemp(float temp);
    void new12VVoltage(float voltage);
    void newOilTemp(float temp);
    void newModuleATemp(float temp);
    void newRequestedTorque(float torque);

  public:
    static constexpr size_t num_of_filters = 1;
    inline static can_filter filters[num_of_filters] = {
        {
            0x0A0,
            0x0CF // Grab all messages from 0xA0 to 0xCF (Everything reserved by the motor controller)
        }
    };

    static constexpr uint32_t timeout_ms = 500;
    static constexpr uint32_t torque_slew_rate = 60; // 60 N.m/s
    std::atomic<float> requestedTorque = 0;
    std::atomic<float> outputTorque = 0;
    
  private:
    void newRequestedTorqueHandler(float torque) {
        requestedTorque = torque;
        emit newRequestedTorque(requestedTorque);
    }
};

} // namespace demo
