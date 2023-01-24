#include <EnergyMeter.hpp>

using namespace CAN;

void EnergyMeter::generateValues() {
    static float voltage = 620;
    static float current = 0;
    emit newCurrent(current += 3.8f);
    emit newVoltage(voltage -= 2.7f);

    if (voltage <= 480) {
        voltage = 620;
    }
    if (current >= 200) {
        current = 0;
    }

    // debug screen
    static float test_value = 0.0f;
    emit onNewTestValue(test_value += 1.0f);
    if (test_value >= 100.0f) {
        test_value = 0.0f;
    }
}