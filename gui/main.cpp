#include <QApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDBusMetaType>
#include <QDBusConnection>
#include <QIcon>
#include "WireguardTypes.h"
#include "WireguardManagerProxy.h"
#include "WireguardManagerBridge.h"
#include "SettingsManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("yet-another-wireguard-gui"));
    app.setApplicationVersion(QStringLiteral("1.0.1"));
    app.setOrganizationName(QStringLiteral("io.github.traciges"));
    app.setWindowIcon(QIcon(QStringLiteral(":/app-icon.png")));

    QCommandLineParser parser;
    parser.addOption({QStringLiteral("tray"), QStringLiteral("Start minimized to system tray")});
    parser.parse(app.arguments());
    const bool startInTray = parser.isSet(QStringLiteral("tray"));

    qDBusRegisterMetaType<ProfileInfo>();
    qDBusRegisterMetaType<ProfileList>();

    IoGithubTracigesWireguardManagerInterface proxy(
        DBusServiceName,
        DBusObjectPath,
        QDBusConnection::systemBus()
    );
    WireguardManagerBridge bridge(&proxy);
    SettingsManager settingsManager;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("wireguardManager"), &bridge);
    engine.rootContext()->setContextProperty(QStringLiteral("settingsManager"), &settingsManager);
    engine.rootContext()->setContextProperty(QStringLiteral("startInTray"), startInTray);

    engine.load(QUrl(QStringLiteral("qrc:/YetAnotherWireguardGui/qml/Main.qml")));

    if (engine.rootObjects().isEmpty())
        return 1;

    return app.exec();
}
