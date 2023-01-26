#pragma once

#include <QObject>

#include <FakeInterface.hpp>

class MotorController : public QObject, public CAN::FakeInterface {
    Q_OBJECT
  public:
    MotorController(const std::string& /* dbc_file_path */ = "") : QObject(nullptr) {
        this->CAN::FakeInterface::startReceiving();
    }
    ~MotorController() = default;

  signals:
    void newMotorRPM(float rpm);
    void newMotorTemp(float temp);
    void newCoolantTemp(float temp);
    void newOilTemp(float temp);
    void new12VVoltage(float voltage);

  private:
    void generateValues();
};