#pragma once

#include <QObject>
#include <fmt/core.h>

#include <FakeInterface.hpp>

class MotorController : public QObject, public CAN::FakeInterface {
    Q_OBJECT
  public:
    MotorController(const std::string& /* dbc_file_path */ = "") : QObject(nullptr) {
        this->CAN::FakeInterface::startReceiving();
    }
    ~MotorController() = default;

  Q_INVOKABLE void clearFaultCodes() {
    fmt::print("Attempt to clear faults\n");
  }

  signals:
    void newMotorRPM(float rpm);
    void newMotorTemp(float temp);
    void newCoolantTemp(float temp);
    void newOilTemp(float temp);
    void new12VVoltage(float voltage);

    // debug screen
    void newTestValue(float test_value);
    void newTestFault(int test_fault);

  private:
    void generateValues();
};