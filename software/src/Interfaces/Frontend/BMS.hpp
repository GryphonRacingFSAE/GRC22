#pragma once

#include <QObject>

#include <Frontend/FakeInterface.hpp>

namespace fake {

class BMS : public QObject, public FakeInterface {
    Q_OBJECT
    Q_PROPERTY(QList<float> voltages MEMBER m_voltages NOTIFY voltagesChanged)
    Q_PROPERTY(QList<float> resistances MEMBER m_resistances NOTIFY resistancesChanged)
  public:
    BMS(const std::string& /* dbc_file_path */ = "") : QObject(nullptr), m_voltages(5 * 28, -40), m_resistances(5 * 28, -40){
        this->FakeInterface::startReceiving();
    }
    ~BMS() = default;

  signals:
    void newAvgPackCurrent(float current);
    void newSignedCurrent(float current);
    void newUnsignedCurrent(float current);
    void newCurrentLimitStatus(float current);
    void newChargeCurrentLimitA(float current);
    void newChargeCurrentLimitKW(float current);
    void newDischargeCurrentLimitA(float current);
    void newDischargeCurrentLimitKW(float current);

    void newVoltage(float voltage);
    void newOpenVoltage(float voltage);
    void newSummedVoltage(float voltage);
    void newAvgCellVoltage(float voltage);
    void newLowCellVoltage(float voltage);
    void newLowCellVoltageID(float voltage);
    void newHighCellVoltage(float voltage);
    void newHighCellVoltageID(float voltage);
    void newAvgCellOpenVoltage(float voltage);
    void newLowCellOpenVoltage(float voltage);
    void newHighCellOpenVoltage(float voltage);
    void newMinCellVoltage(float voltage);
    void newMaxCellVoltage(float voltage);

    void newAvgTemp(float temp);
    void newLowestTemp(float temp);
    void newHighestTemp(float temp);
    void newHeatsinkTemp(float temp);
    void newHottestThermistorID(float temp);

    void newStateOfCharge(float value);
    void newAmphours(float value);
    void newResistance(float value);
    void newHealth(float value);
    void newTotalPackCycles(float value);
    void newPackPowerKW(float value);

    void newCellVoltage(int segment, int id, float voltage);
    void voltagesChanged();

    void newCellResistance(int segment, int id, float ohms);
    void resistancesChanged();

  private:
    QList<float> m_voltages;
    QList<float> m_resistances;

  private:
    void generateValues();
};

} // namespace fake