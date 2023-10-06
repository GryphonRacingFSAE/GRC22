#pragma once

#include <FakeInterface.hpp>
#include <QObject>
#include <string>

class AN400ECU : public QObject, public CAN::FakeInterface {
    Q_OBJECT
  public:
    AN400ECU(const std::string& /* dbc_file_path */ = "") : QObject(nullptr) {
        this->CAN::FakeInterface::startReceiving();
    }
    ~AN400ECU() = default;

  signals:
    void newMAP(float map);
    void newLambda(float lambda);
    void newTPS(float tps);
    void newBatteryVoltage(float voltage);
    void newAirTemp(float temp);
    void newCoolantTemp(float temp);
    void newRPM(float rpm);

  private:
    void generateValues();
};
