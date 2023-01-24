#pragma once

#include <FakeInterface.hpp>
#include <QObject>

class AN400ECU : public QObject, public CAN::FakeInterface {
    Q_OBJECT
  public:
    AN400ECU(QObject* parent = nullptr) : QObject(parent) {
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
