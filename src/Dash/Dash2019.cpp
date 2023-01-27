#include <QApplication>
#include <QQmlApplicationEngine>
#include <fmt/core.h>
#include <memory>

#include <AN400ECU.hpp>

#ifdef DEBUG
#include <runtimeqml.hpp>
#endif

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    std::string app_root = app.applicationDirPath().toStdString();
    auto ecu =
        std::make_unique<AN400ECU>(app_root + "/AN400ECU.dbc"); // Should be before QQml engine
    QQmlApplicationEngine engine;

    qmlRegisterSingletonInstance<AN400ECU>("CAN.AN400ECU", 1, 0, "AN400ECU", ecu.get());
#ifdef DEBUG
    RuntimeQml* rt = new RuntimeQml(&engine);
    rt->parseQrc(ROOT_SOURCE_PATH "/qml.qrc");
    rt->setAutoReload(true); // Reload automatically on file update

    rt->load(QStringLiteral("qrc:/main2019.qml"));
#else
    engine.load(QStringLiteral("qrc:/main2019.qml"));
#endif
    return app.exec();
}
