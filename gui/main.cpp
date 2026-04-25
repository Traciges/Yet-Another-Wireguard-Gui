#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDBusMetaType>
#include <QDBusConnection>
#include <QIcon>
#include "WireguardTypes.h"
#include "WireguardManagerProxy.h"
#include "WireguardManagerBridge.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("yet-another-wireguard-gui"));
    app.setOrganizationName(QStringLiteral("example"));
    app.setWindowIcon(QIcon(QStringLiteral(":/app-icon.png")));

    qDBusRegisterMetaType<ProfileInfo>();
    qDBusRegisterMetaType<ProfileList>();

    OrgExampleWireguardManagerInterface proxy(
        QStringLiteral("org.example.WireguardManager"),
        QStringLiteral("/org/example/WireguardManager"),
        QDBusConnection::sessionBus()
    );
    WireguardManagerBridge bridge(&proxy);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(
        QStringLiteral("wireguardManager"), &bridge
    );

    engine.loadFromModule("YetAnotherWireguardGui", "Main");

    if (engine.rootObjects().isEmpty()) {
        return 1;
    }

    return app.exec();
}
