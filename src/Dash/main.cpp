#include <QApplication>
#include <QQmlApplicationEngine>
#include <fmt/core.h>

#include <BMS.hpp>
#include <EnergyMeter.hpp>
#include <MotorController.hpp>

#ifdef QT_DEBUG
#include <runtimeqml.hpp>
#endif

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    std::string app_root = app.applicationDirPath().toStdString();
    // Should be before QQml engine
    MotorController* motor_controller = new MotorController(app_root + "/20220510_Gen5_CAN_DB.dbc"); 
    EnergyMeter* energy_meter = new EnergyMeter(app_root + "/Energy_Meter_CAN_Messages.dbc");
    BMS* bms = new BMS(app_root + "/Orion_CANBUS.dbc");

    QQmlApplicationEngine engine;

    qmlRegisterSingletonInstance<MotorController>(
        "CAN.MotorController", 1, 0, "MotorController", motor_controller);
    qmlRegisterSingletonInstance<EnergyMeter>("CAN.EnergyMeter", 1, 0, "EnergyMeter", energy_meter);
    qmlRegisterSingletonInstance<BMS>("CAN.BMS", 1, 0, "BMS", bms);
#ifdef QT_DEBUG
    RuntimeQml* rt = new RuntimeQml(&engine);
    rt->parseQrc(ROOT_SOURCE_PATH "/qml.qrc");
    rt->setAutoReload(true); // Reload automatically on file update

    rt->load(QStringLiteral("qrc:/main.qml"));
#else
    engine.load(QStringLiteral("qrc:/main.qml"));
#endif
    return app.exec();
}
