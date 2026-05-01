#include "WireguardManagerBridge.h"
#include "WireguardTypes.h"
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTimer>
#include <QVariantMap>

WireguardManagerBridge::WireguardManagerBridge(
    IoGithubTracigesWireguardManagerInterface *proxy, QObject *parent)
    : QObject(parent), m_proxy(proxy)
{
    connect(m_proxy, &IoGithubTracigesWireguardManagerInterface::ProfileStatusChanged,
            this, &WireguardManagerBridge::profileStatusChanged);
    connect(m_proxy, &IoGithubTracigesWireguardManagerInterface::ProfileImported,
            this, &WireguardManagerBridge::profileImported);
    connect(m_proxy, &IoGithubTracigesWireguardManagerInterface::ProfileDeleted,
            this, &WireguardManagerBridge::profileDeleted);
    connect(m_proxy, &IoGithubTracigesWireguardManagerInterface::ProfileRenamed,
            this, &WireguardManagerBridge::profileRenamed);
    connect(m_proxy, &IoGithubTracigesWireguardManagerInterface::ErrorOccurred,
            this, &WireguardManagerBridge::errorOccurred);
}

void WireguardManagerBridge::refreshProfiles()
{
    auto *watcher = new QDBusPendingCallWatcher(m_proxy->ListProfiles(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *w) {
                w->deleteLater();
                QDBusPendingReply<ProfileList> reply = *w;
                if (reply.isError()) {
                    const QDBusError::ErrorType type = reply.error().type();
                    // Daemon may not be ready yet during autostart — retry silently
                    if ((type == QDBusError::ServiceUnknown || type == QDBusError::NoReply)
                            && m_startupRetries < MaxStartupRetries) {
                        m_startupRetries++;
                        QTimer::singleShot(2000, this, &WireguardManagerBridge::refreshProfiles);
                        return;
                    }
                    m_startupRetries = 0;
                    if (type == QDBusError::ServiceUnknown || type == QDBusError::NoReply) {
                        m_daemonUnavailable = true;
                        emit daemonUnavailable();
                    } else {
                        emit errorOccurred(QString(), reply.error().message());
                    }
                    return;
                }
                m_startupRetries = 0;
                m_daemonUnavailable = false;
                QVariantList result;
                for (const ProfileInfo &info : reply.value()) {
                    QVariantMap entry;
                    entry[QStringLiteral("name")] = info.name;
                    entry[QStringLiteral("status")] = info.status;
                    entry[QStringLiteral("rxBytes")] = info.rxBytes;
                    entry[QStringLiteral("txBytes")] = info.txBytes;
                    result.append(entry);
                }
                emit profilesLoaded(result);
            });
}

void WireguardManagerBridge::toggleProfile(const QString &name, bool targetState)
{
    m_proxy->ToggleProfile(name, targetState);
}

void WireguardManagerBridge::importProfile(const QUrl &fileUrl)
{
    const QString path = fileUrl.toLocalFile();
    const QString name = QFileInfo(path).completeBaseName();

    static const QRegularExpression nameRegex(QStringLiteral("^[a-zA-Z0-9_=+.-]{1,15}$"));
    if (!nameRegex.match(name).hasMatch()) {
        emit errorOccurred(QString(), QStringLiteral("Invalid filename: \"%1\"").arg(name));
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred(QString(), QStringLiteral("Could not open file: %1")
                           .arg(file.errorString()));
        return;
    }

    const QByteArray data = file.read(65537);
    if (data.size() > 65536) {
        emit errorOccurred(QString(), QStringLiteral("Configuration file too large (max. 64 KB)"));
        return;
    }

    m_proxy->ImportProfile(name, QString::fromUtf8(data));
}

void WireguardManagerBridge::deleteProfile(const QString &name)
{
    m_proxy->DeleteProfile(name);
}

void WireguardManagerBridge::renameProfile(const QString &oldName, const QString &newName)
{
    static const QRegularExpression nameRegex(QStringLiteral("^[a-zA-Z0-9_=+.-]{1,15}$"));
    if (!nameRegex.match(oldName).hasMatch()) {
        emit errorOccurred(oldName, QStringLiteral("Invalid profile name: \"%1\"").arg(oldName));
        return;
    }
    if (!nameRegex.match(newName).hasMatch()) {
        emit errorOccurred(oldName, QStringLiteral("Invalid new profile name: \"%1\"").arg(newName));
        return;
    }
    if (oldName == newName)
        return;
    m_proxy->RenameProfile(oldName, newName);
}

static QDBusMessage systemdMsg(const QString &method)
{
    return QDBusMessage::createMethodCall(
        QStringLiteral("org.freedesktop.systemd1"),
        QStringLiteral("/org/freedesktop/systemd1"),
        QStringLiteral("org.freedesktop.systemd1.Manager"),
        method);
}

void WireguardManagerBridge::enableAndStartDaemon()
{
    // setInteractiveAuthorizationAllowed(true) tells polkit to show an auth dialog
    QDBusMessage enableMsg = systemdMsg(QStringLiteral("EnableUnitFiles"));
    enableMsg << QStringList{QStringLiteral("yawg-daemon.service")} << false << false;
    enableMsg.setInteractiveAuthorizationAllowed(true);

    auto *enableWatcher = new QDBusPendingCallWatcher(
        QDBusConnection::systemBus().asyncCall(enableMsg), this);

    connect(enableWatcher, &QDBusPendingCallWatcher::finished, this,
        [this](QDBusPendingCallWatcher *w) {
            w->deleteLater();
            if (w->isError()) {
                emit errorOccurred(QString(),
                    QStringLiteral("Failed to enable daemon: %1").arg(w->error().message()));
                return;
            }

            QDBusMessage reloadMsg = systemdMsg(QStringLiteral("Reload"));
            reloadMsg.setInteractiveAuthorizationAllowed(true);

            auto *reloadWatcher = new QDBusPendingCallWatcher(
                QDBusConnection::systemBus().asyncCall(reloadMsg), this);

            connect(reloadWatcher, &QDBusPendingCallWatcher::finished, this,
                [this](QDBusPendingCallWatcher *rw) {
                    rw->deleteLater();
                    if (rw->isError()) {
                        emit errorOccurred(QString(),
                            QStringLiteral("Failed to reload systemd: %1").arg(rw->error().message()));
                        return;
                    }

                    QDBusMessage startMsg = systemdMsg(QStringLiteral("StartUnit"));
                    startMsg << QStringLiteral("yawg-daemon.service") << QStringLiteral("replace");
                    startMsg.setInteractiveAuthorizationAllowed(true);

                    auto *startWatcher = new QDBusPendingCallWatcher(
                        QDBusConnection::systemBus().asyncCall(startMsg), this);

                    connect(startWatcher, &QDBusPendingCallWatcher::finished, this,
                        [this](QDBusPendingCallWatcher *sw) {
                            sw->deleteLater();
                            if (sw->isError()) {
                                emit errorOccurred(QString(),
                                    QStringLiteral("Failed to start daemon: %1").arg(sw->error().message()));
                                return;
                            }
                            m_startupRetries = 0;
                            m_daemonUnavailable = false;
                            QTimer::singleShot(1000, this, &WireguardManagerBridge::refreshProfiles);
                        });
                });
        });
}

void WireguardManagerBridge::exportProfile(const QString &name, const QUrl &fileUrl)
{
    auto *watcher = new QDBusPendingCallWatcher(m_proxy->ExportProfile(name), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, name, fileUrl](QDBusPendingCallWatcher *w) {
                w->deleteLater();
                QDBusPendingReply<QString> reply = *w;
                if (reply.isError()) {
                    emit errorOccurred(name, reply.error().message());
                    return;
                }

                const QString path = fileUrl.toLocalFile();
                QFile file(path);
                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    emit errorOccurred(name, QStringLiteral("Could not write file: %1")
                                       .arg(file.errorString()));
                    return;
                }
                file.write(reply.value().toUtf8());
                file.close();

                emit profileExported(name);
            });
}

void WireguardManagerBridge::addProfile(
    const QString &name, const QString &privateKey, const QString &address,
    const QString &dns, const QString &mtu, const QString &publicKey,
    const QString &presharedKey, const QString &allowedIPs, const QString &endpoint)
{
    static const QRegularExpression nameRegex(QStringLiteral("^[a-zA-Z0-9_=+.-]{1,15}$"));
    if (!nameRegex.match(name).hasMatch()) {
        emit errorOccurred(QString(), QStringLiteral("Invalid profile name: \"%1\"").arg(name));
        return;
    }

    // Strip embedded newlines from all values to prevent config file corruption
    auto clean = [](const QString &s) {
        return QString(s).remove(u'\n').remove(u'\r');
    };

    QString contents = QStringLiteral("[Interface]\n");
    contents += QStringLiteral("PrivateKey = %1\n").arg(clean(privateKey));
    contents += QStringLiteral("Address = %1\n").arg(clean(address));
    if (!dns.isEmpty())
        contents += QStringLiteral("DNS = %1\n").arg(clean(dns));
    if (!mtu.isEmpty())
        contents += QStringLiteral("MTU = %1\n").arg(clean(mtu));
    contents += QStringLiteral("\n[Peer]\n");
    contents += QStringLiteral("PublicKey = %1\n").arg(clean(publicKey));
    if (!presharedKey.isEmpty())
        contents += QStringLiteral("PresharedKey = %1\n").arg(clean(presharedKey));
    contents += QStringLiteral("AllowedIPs = %1\n").arg(clean(allowedIPs));
    contents += QStringLiteral("Endpoint = %1\n").arg(clean(endpoint));

    if (contents.toUtf8().size() > 65536) {
        emit errorOccurred(QString(), QStringLiteral("Configuration too large (max. 64 KB)"));
        return;
    }

    m_proxy->ImportProfile(name, contents);
}
