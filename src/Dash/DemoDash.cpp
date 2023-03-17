#include <QApplication>
#include <QQmlApplicationEngine>
#include <fmt/core.h>

#include <Frontend/BMS.hpp>
#include <Frontend/EnergyMeter.hpp>
#include <Demo/MotorController.hpp>
#include <Full/VCU.hpp>

#ifdef DEBUG
#include <runtimeqml.hpp>
#endif

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    std::string app_root = app.applicationDirPath().toStdString();
    // Should be before QQml engine
    demo::MotorController* motor_controller =
        new demo::MotorController(app_root + "/DBCs/20220510_Gen5_CAN_DB.dbc");
    fake::EnergyMeter* energy_meter = new fake::EnergyMeter(app_root + "/DBCs/Energy_Meter_CAN_Messages.dbc");
    fake::BMS* bms = new fake::BMS(app_root + "/DBCs/Orion_CANBUS.dbc");
    real::VCU* vcu = new real::VCU(app_root + "/TorqueMaps");

    QQmlApplicationEngine engine;

    qmlRegisterSingletonInstance<demo::MotorController>(
        "CAN.MotorController", 1, 0, "MotorController", motor_controller);
    qmlRegisterSingletonInstance<fake::EnergyMeter>("CAN.EnergyMeter", 1, 0, "EnergyMeter", energy_meter);
    qmlRegisterSingletonInstance<fake::BMS>("CAN.BMS", 1, 0, "BMS", bms);
    qmlRegisterSingletonInstance<real::VCU>("CAN.VCU", 1, 0, "VCU", vcu);
#ifdef DEBUG
    RuntimeQml* rt = new RuntimeQml(&engine);
    rt->parseQrc(ROOT_SOURCE_PATH "/qml.qrc");
    rt->setAutoReload(true); // Reload automatically on file update

    rt->load(QStringLiteral("qrc:/demo.qml"));
#else
    engine.load(QStringLiteral("qrc:/demo.qml"));
#endif
    auto retcode = app.exec();
    return retcode;
}
