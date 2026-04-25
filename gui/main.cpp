#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDBusMetaType>
#include "WireguardTypes.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("yet-another-wireguard-gui"));
    app.setOrganizationName(QStringLiteral("example"));

    qDBusRegisterMetaType<ProfileInfo>();
    qDBusRegisterMetaType<ProfileList>();

    QQmlApplicationEngine engine;
    engine.loadFromModule("YetAnotherWireguardGui", "Main");

    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    return app.exec();
}
