#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtGlobal>
#include <fmt/core.h>

#include <BMS.hpp>
#include <EnergyMeter.hpp>
#include <MotorController.hpp>

#ifdef DEBUG
#include <runtimeqml.hpp>
#endif

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    std::string app_root = app.applicationDirPath().toStdString();
    // Should be before QQml engine
<<<<<<< HEAD
    auto motor_controller =
        std::make_unique<MotorController>(app_root + "/20220510_Gen5_CAN_DB.dbc");
    auto energy_meter = std::make_unique<EnergyMeter>(app_root + "/Energy_Meter_CAN_Messages.dbc");
    auto bms = std::make_unique<BMS>(app_root + "/Orion_CANBUS.dbc");
=======
    MotorController* motor_controller = new MotorController(app_root + "/20220510_Gen5_CAN_DB.dbc");
    EnergyMeter* energy_meter = new EnergyMeter(app_root + "/Energy_Meter_CAN_Messages.dbc");
    BMS* bms = new BMS(app_root + "/Orion_CANBUS.dbc");
>>>>>>> parent of c6ac3bf (Change to unique_ptr and fix packaging on rpi)

    QQmlApplicationEngine engine;

    qmlRegisterSingletonInstance<MotorController>(
<<<<<<< HEAD
        "CAN.MotorController", 1, 0, "MotorController", motor_controller.get());
    qmlRegisterSingletonInstance<EnergyMeter>(
        "CAN.EnergyMeter", 1, 0, "EnergyMeter", energy_meter.get());
    qmlRegisterSingletonInstance<BMS>("CAN.BMS", 1, 0, "BMS", bms.get());
=======
        "CAN.MotorController", 1, 0, "MotorController", motor_controller);
    qmlRegisterSingletonInstance<EnergyMeter>("CAN.EnergyMeter", 1, 0, "EnergyMeter", energy_meter);
    qmlRegisterSingletonInstance<BMS>("CAN.BMS", 1, 0, "BMS", bms);
>>>>>>> parent of c6ac3bf (Change to unique_ptr and fix packaging on rpi)
#ifdef DEBUG
    RuntimeQml* rt = new RuntimeQml(&engine);
    rt->parseQrc(ROOT_SOURCE_PATH "/qml.qrc");
    rt->setAutoReload(true); // Reload automatically on file update

    rt->load(QStringLiteral("qrc:/main.qml"));
#else
    engine.load(QStringLiteral("qrc:/main.qml"));
#endif
    return app.exec();
}
