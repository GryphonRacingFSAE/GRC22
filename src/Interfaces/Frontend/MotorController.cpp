#include <MotorController.hpp>

using namespace CAN;

void MotorController::generateValues() {
    static float rpm = 0.0f;
    static float oil_temp = -10;
    static float coolant_temp = -14;
    static float motor_temp = -13;
    static float voltage = 8.00f;
    emit newMotorRPM(rpm);
    emit newCoolantTemp(coolant_temp);
    emit newOilTemp(oil_temp);
    emit newMotorTemp(motor_temp);
    emit new12VVoltage(voltage);

    rpm += 50.0f;
    if (rpm >= 5000) {
        rpm = 0.0f;
    }
    oil_temp += 2.3f;
    coolant_temp += 1.7f;
    motor_temp += 1.2f;
    if (oil_temp >= 80) {
        oil_temp = -10;
    }
    if (coolant_temp >= 70) {
        coolant_temp = -10;
    }
    if (motor_temp >= 90) {
        motor_temp = -10;
    }
    voltage += 0.15f;
    if (voltage >= 14.5f) {
        voltage = 8.00f;
    }

    // debug screen
    static float test_value = 0.0f;
    emit newTestValue(test_value += 1.0f);
    if (test_value >= 100.0f) {
        test_value = 0.0f;
    }
}