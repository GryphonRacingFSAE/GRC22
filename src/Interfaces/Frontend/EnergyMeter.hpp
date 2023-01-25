#pragma once

#include <FakeInterface.hpp>
#include <QObject>

class EnergyMeter : public QObject, public CAN::FakeInterface {
    Q_OBJECT
  public:
    EnergyMeter(const std::string& /* dbc_file_path */) : QObject(nullptr) {
        this->CAN::FakeInterface::startReceiving();
    }
    ~EnergyMeter() = default;

  signals:
    void newVoltage(float voltage);
    void newCurrent(float current);

  private:
    void generateValues();
};