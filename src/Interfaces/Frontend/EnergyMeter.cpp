#include <EnergyMeter.hpp>

using namespace CAN;

void EnergyMeter::generateValues() {
    static float voltage = 0.0f;
    static float current = 0.0f;

    emit newCurrent(current += 0.1f);
    emit newVoltage(voltage += 0.1f);

    if (voltage >= 100) {
        voltage = 0.0f;
    }
    if (current >= 100) {
        current = 0.0f;
    }
}