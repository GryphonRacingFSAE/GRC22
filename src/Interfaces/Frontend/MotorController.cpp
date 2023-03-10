#include <MotorController.hpp>

using namespace CAN;

void MotorController::generateValues() {
    static float current = 0.0f;
    static float voltage = 0.0f;
    static float temp = 0.0f;
    static float internal_state = 0.0f;
    static float analog_voltage = 0.0f;
    static float torque = 0.0f;
    static float speed = 0.0f;
    static float post_fault = 0.0f;
    static float run_fault = 0.0f;

    // DEBUG A
    emit newDCBusCurrent(current);

    emit newDCBusVoltage(voltage);
    emit newOutputVoltage(voltage);

    emit newModuleATemp(temp);
    emit newModuleBTemp(temp);
    emit newModuleCTemp(temp);
    emit newGateDriverBoardTemp(temp);
    emit newControlBoardTemp(temp);
    emit newCoolantTemp(temp);
    emit newHotSpotTemp(temp);
    emit newMotorTemp(temp);

    // DEBUG B
    emit newPWMFrequency(internal_state);
    emit newInverterState(internal_state);
    emit newInverterRunMode(internal_state);
    emit newInverterActiveDischargeState(internal_state);
    emit newInverterEnableLockout(internal_state);
    emit newBMSActive(internal_state);
    emit newBMSLimitingTorque(internal_state);
    emit newLimitMaxSpeed(internal_state);
    emit newLimitHotSpot(internal_state);
    emit newLowSpeedLimiting(internal_state);
    emit newCoolantTempLimiting(internal_state);

    emit newAnalogInput1(analog_voltage);
    emit newAnalogInput2(analog_voltage);
    emit newAnalogInput3(analog_voltage);
    emit newAnalogInput4(analog_voltage);
    emit newAnalogInput5(analog_voltage);
    emit newAnalogInput6(analog_voltage);

    emit newTorqueShudder(torque);
    emit newCommandedTorque(torque);
    emit newTorqueFeedback(torque);

    emit newMotorSpeed(speed);

    // DEBUG C
    emit newPOSTFaultHigh(post_fault);
    emit newPOSTFaultLow(post_fault);

    // DEBUG D
    emit newRUNFaultHigh(run_fault);
    emit newRUNFaultLow(run_fault);

    current += 0.1f;
    voltage += 0.1f;
    temp += 0.1f;
    internal_state += 0.1f;
    analog_voltage += 0.1f;
    torque += 0.1f;
    speed += 0.1f;

    if (current >= 100) {
        current = 0.0f;
    }
    if (voltage >= 100) {
        voltage = 0.0f;
    }
    if (temp >= 100) {
        temp = 0.0f;
    }
    if (internal_state >= 100) {
        internal_state = 0.0f;
    }
    if (analog_voltage >= 100) {
        analog_voltage = 0.0f;
    }
    if (torque >= 100) {
        torque = 0.0f;
    }
    if (speed >= 100) {
        speed = 0.0f;
    }

    if (post_fault == 0) {
        post_fault = 1.0f;
    } else {
        post_fault = 0.0f;
    }

    if (run_fault == 0) {
        run_fault = 1.0f;
    } else {
        run_fault = 0.0f;
    }
}