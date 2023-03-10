#pragma once

#include <DBCInterface.hpp>
#include <QObject>

class MotorController : public QObject, public CAN::DBCInterface<MotorController> {
    Q_OBJECT
  public:
    MotorController(const std::string& dbc_file_path = "20220510_Gen5_CAN_DB.dbc")
        : QObject(nullptr), DBCInterface(dbc_file_path) {
        // DRIVER
        can_signal_dispatch["INV_Motor_Speed"] = &MotorController::newMotorRPM;
        can_signal_dispatch["INV_Motor_Temp"] = &MotorController::newMotorTemp;
        can_signal_dispatch["INV_Coolant_Temp"] = &MotorController::newCoolantTemp;
        can_signal_dispatch["INV_Analog_Input_1"] = &MotorController::new12VVoltage;
        can_signal_dispatch["INV_Analog_Input_2"] = &MotorController::newOilTemp;
        can_signal_dispatch["INV_Module_A_Temp"] = &MotorController::newModuleATemp;

        // DEBUG
        can_signal_dispatch["INV_DC_Bus_Current"] = &MotorController::newDCBusCurrent;

        can_signal_dispatch["INV_DC_Bus_Voltage"] = &MotorController::newDCBusVoltage;
        can_signal_dispatch["INV_Output_Voltage"] = &MotorController::newOutputVoltage;

        // can_signal_dispatch["INV_Module_A_Temp"] = &MotorController::newModuleATemp;
        can_signal_dispatch["INV_Module_B_Temp"] = &MotorController::newModuleBTemp;
        can_signal_dispatch["INV_Module_C_Temp"] = &MotorController::newModuleCTemp;
        can_signal_dispatch["INV_Gate_Driver_Board_Temp"] =
            &MotorController::newGateDriverBoardTemp;
        can_signal_dispatch["INV_Control_Board_Temp"] = &MotorController::newControlBoardTemp;
        // can_signal_dispatch["INV_Coolant_Temp"] = &MotorController::newCoolantTemp;
        can_signal_dispatch["INV_Hot_Spot_Temp"] = &MotorController::newHotSpotTemp;
        // can_signal_dispatch["INV_Motor_Temp"] = &MotorController::newMotorTemp;

        can_signal_dispatch["INV_PWM_Frequency"] = &MotorController::newPWMFrequency;
        can_signal_dispatch["INV_Inverter_State"] = &MotorController::newInverterState;
        can_signal_dispatch["INV_Inverter_Run_Mode"] = &MotorController::newInverterRunMode;
        can_signal_dispatch["INV_Inverter_Discharge_State"] =
            &MotorController::newInverterActiveDischargeState;
        can_signal_dispatch["INV_Inverter_Enable_Lockout"] =
            &MotorController::newInverterEnableLockout;
        can_signal_dispatch["INV_BMS_Active"] = &MotorController::newBMSActive;
        can_signal_dispatch["INV_BMS_Limiting_Torque"] = &MotorController::newBMSLimitingTorque;
        can_signal_dispatch["INV_Limit_Max_Speed"] = &MotorController::newLimitMaxSpeed;
        can_signal_dispatch["INV_Limit_Hot_Spot"] = &MotorController::newLimitHotSpot;
        can_signal_dispatch["INV_Low_Speed_Limiting"] = &MotorController::newLowSpeedLimiting;
        can_signal_dispatch["INV_Limit_Coolant_Derating"] =
            &MotorController::newCoolantTempLimiting;

        can_signal_dispatch["INV_Analog_Input_1"] = &MotorController::newAnalogInput1;
        can_signal_dispatch["INV_Analog_Input_2"] = &MotorController::newAnalogInput2;
        can_signal_dispatch["INV_Analog_Input_3"] = &MotorController::newAnalogInput3;
        can_signal_dispatch["INV_Analog_Input_4"] = &MotorController::newAnalogInput4;
        can_signal_dispatch["INV_Analog_Input_5"] = &MotorController::newAnalogInput5;
        can_signal_dispatch["INV_Analog_Input_6"] = &MotorController::newAnalogInput6;

        can_signal_dispatch["INV_Torque_Shudder"] = &MotorController::newTorqueShudder;
        can_signal_dispatch["INV_Commanded_Torque"] = &MotorController::newCommandedTorque;
        can_signal_dispatch["INV_Torque_Feedback"] = &MotorController::newTorqueFeedback;

        can_signal_dispatch["INV_Motor_Speed"] = &MotorController::newMotorSpeed;
    }

    Q_INVOKABLE void clearFaultCodes() {
        // CM200 CAN Protocol V5.9 Section 2.3.1 and 2.3.3 (Address 20 or 0x14)
        RetCode ans =
            write(0x0C1, std::array<uint8_t, 8>{0x00, 0x14, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00});

        if (ans != RetCode::Success) {
            fmt::print("Failed to clear fault codes\n");
        }
    }

  signals:
    // DRIVER
    void newMotorRPM(float rpm);
    void newMotorTemp(float temp);
    void newCoolantTemp(float temp);
    void new12VVoltage(float voltage);
    void newOilTemp(float temp);
    void newModuleATemp(float temp);

    // DEBUG
    void newDCBusCurrent(float current);

    void newDCBusVoltage(float voltage);
    void newOutputVoltage(float voltage);

    // void newModuleATemp(float temp);
    void newModuleBTemp(float temp);
    void newModuleCTemp(float temp);
    void newGateDriverBoardTemp(float temp);
    void newControlBoardTemp(float temp);
    // void newCoolantTemp(float temp);
    void newHotSpotTemp(float temp);
    // void newMotorTemp(float temp);

    void newPWMFrequency(float state);
    void newInverterState(float state);
    void newInverterRunMode(float state);
    void newInverterActiveDischargeState(float state);
    void newInverterEnableLockout(float state);
    void newBMSActive(float state);
    void newBMSLimitingTorque(float state);
    void newLimitMaxSpeed(float state);
    void newLimitHotSpot(float state);
    void newLowSpeedLimiting(float state);
    void newCoolantTempLimiting(float state);

    void newAnalogInput1(float voltage);
    void newAnalogInput2(float voltage);
    void newAnalogInput3(float voltage);
    void newAnalogInput4(float voltage);
    void newAnalogInput5(float voltage);
    void newAnalogInput6(float voltage);

    void newTorqueShudder(float torque);
    void newCommandedTorque(float torque);
    void newTorqueFeedback(float torque);

    void newMotorSpeed(float speed);

  public:
    static constexpr size_t num_of_filters = 2;
    inline static can_filter filters[num_of_filters] = {
        {
            0x0A0,
            0x7F0 // Grab all messages from 0xA0 to 0xAF
        },
        {
            0x0B0,
            0x7FF // Grab all messages from 0xB0
        }};

    static constexpr uint32_t timeout_ms = 500;
};
