#include <Frontend/BMS.hpp>

void fake::BMS::generateValues() {
    static float voltage = 0.0f;
    static float current = 0.0f;
    static float temp = 0.0f;
    static float value = 0.0f;

    emit newAvgPackCurrent(current);
    emit newSignedCurrent(current);
    emit newUnsignedCurrent(current);
    emit newCurrentLimitStatus(current);
    emit newChargeCurrentLimitA(current);
    emit newChargeCurrentLimitKW(current);
    emit newDischargeCurrentLimitA(current);
    emit newDischargeCurrentLimitKW(current);

    emit newVoltage(voltage);
    emit newOpenVoltage(voltage);
    emit newSummedVoltage(voltage);
    emit newAvgCellVoltage(voltage);
    emit newLowCellVoltage(voltage);
    emit newLowCellVoltageID(voltage);
    emit newHighCellVoltage(voltage);
    emit newHighCellVoltageID(voltage);
    emit newAvgCellOpenVoltage(voltage);
    emit newLowCellOpenVoltage(voltage);
    emit newHighCellOpenVoltage(voltage);
    emit newMinCellVoltage(voltage);
    emit newMaxCellVoltage(voltage);

    emit newAvgTemp(temp);
    emit newLowestTemp(temp);
    emit newHighestTemp(temp);
    emit newHeatsinkTemp(temp);
    emit newHottestThermistorID(temp);

    emit newStateOfCharge(value);
    emit newAmphours(value);
    emit newResistance(value);
    emit newHealth(value);
    emit newTotalPackCycles(value);
    emit newPackPowerKW(value);

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 28; j++) {
            int num = rand() % 5;
            m_voltages[i * 28 + j] = num;
            m_resistances[i * 28 + j] = num;

            emit newCellVoltage(i, j, num);
            emit newCellResistance(i, j, num);
        }
    }
    emit resistancesChanged();
    emit voltagesChanged();

    voltage += 0.1f;
    current += 0.1f;
    temp += 0.1f;
    value += 0.1f;

    if (voltage >= 100) {
        voltage = 0.0f;
    }
    if (current >= 100) {
        current = 0.0f;
    }
    if (temp >= 100) {
        temp = 0.0f;
    }
    if (value >= 100) {
        value = 0.0f;
    }
}