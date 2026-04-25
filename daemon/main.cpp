#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include "WireguardManager.h"
#include "wireguardmanageradaptor.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("yawg-daemon"));

    qDBusRegisterMetaType<ProfileInfo>();
    qDBusRegisterMetaType<ProfileList>();

    WireguardManager manager;
    new WireguardManagerAdaptor(&manager);

    QDBusConnection bus = QDBusConnection::systemBus();

    if (!bus.registerObject(QStringLiteral("/org/example/WireguardManager"), &manager)) {
        qCritical() << "D-Bus object registration failed:" << bus.lastError().message();
        return 1;
    }
    if (!bus.registerService(QStringLiteral("org.example.WireguardManager"))) {
        qCritical() << "D-Bus service registration failed:" << bus.lastError().message();
        return 1;
    }

    qInfo() << "yawg-daemon running on system bus";
    return app.exec();
}
