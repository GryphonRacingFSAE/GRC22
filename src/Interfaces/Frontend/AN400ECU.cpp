#include <AN400ECU.hpp>

using namespace CAN;

void AN400ECU::generateValues() {
    static float voltage = 10.2f;
    static float map = 0;
    static float lambda = 0;
    static float tps = 0;
    static float rpm = 0;
    static float temp = -15;
    
    emit newMAP(map += 0.8f);
    emit newLambda(lambda += 0.2f);
    emit newTPS(tps += 0.3f);
    emit newBatteryVoltage(voltage += 0.2f);
    emit newAirTemp(temp += 1.3f);
    emit newCoolantTemp(temp += 1.4f);
    emit newRPM(rpm += 50.0f);

    if (temp >= 30) {
        temp = -15;
    }
    if (lambda >= 12) {
        lambda = 0;
    }
    if (tps >= 10) {
        tps = 0;
    }
    if (map >= 15) {
        map = 0;
    }
    if (voltage > 14.5) {
        voltage = 10.2f;
    }
    if (rpm >= 5000) {
        rpm = 0.0f;
    }
}