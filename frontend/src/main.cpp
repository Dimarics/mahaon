#include "appglobal.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QDebug>
#include <QGeoServiceProvider>

int main(int argc, char *argv[]) {
    //qputenv("QT_ENABLE_HIGHDPI_SCALING", "0");
    QGuiApplication app(argc, argv);
    // QStringList providers = QGeoServiceProvider::availableServiceProviders();
    // qDebug() << "Доступные провайдеры:" << providers;
    //qputenv("QT_WASM_PLATFORM_PLUGIN", "wasm");
    //qDebug() << QDir(":/").entryList();
    qmlRegisterSingletonType<AppGlobal>("App", 1, 0, "App", AppGlobal::qmlInstance);
    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:/src/");
    engine.load("qrc:/src/main.qml");
    return app.exec();
}
