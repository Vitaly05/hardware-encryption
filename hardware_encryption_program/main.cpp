#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

#include "backend.h"
#include "sodium.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQuickStyle::setStyle("Fusion");

    QQmlApplicationEngine engine;

    qmlRegisterType<Backend>("backend", 1, 0, "Backend");

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("hardware_encryption_program", "Main");

    if (sodium_init() < 0) {
        return 1;
    }

    return QApplication::exec();
}
