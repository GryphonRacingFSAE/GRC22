#pragma once

#include <FakeInterface.hpp>
#include <QObject>

class EnergyMeter : public QObject, public CAN::FakeInterface {
    Q_OBJECT
  public:
    EnergyMeter(QObject* parent = nullptr) : QObject(parent) {
        this->CAN::FakeInterface::startReceiving();
    }
    ~EnergyMeter() = default;

  signals:
    void newVoltage(float voltage);
    void newCurrent(float current);

    // debug screen
    void newTestValue(float test_value);

  private:
    void generateValues();
};