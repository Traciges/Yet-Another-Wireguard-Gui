#include "WireguardManagerBridge.h"
#include "WireguardTypes.h"
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
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
                    emit errorOccurred(QString(), reply.error().message());
                    return;
                }
                QVariantList result;
                for (const ProfileInfo &info : reply.value()) {
                    QVariantMap entry;
                    entry[QStringLiteral("name")] = info.name;
                    entry[QStringLiteral("status")] = info.status;
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
        emit errorOccurred(QString(), QStringLiteral("Ungültiger Dateiname: \"%1\"").arg(name));
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred(QString(), QStringLiteral("Datei konnte nicht geöffnet werden: %1")
                           .arg(file.errorString()));
        return;
    }

    const QByteArray data = file.read(65537);
    if (data.size() > 65536) {
        emit errorOccurred(QString(), QStringLiteral("Konfigurationsdatei zu groß (max. 64 KB)"));
        return;
    }

    m_proxy->ImportProfile(name, QString::fromUtf8(data));
}

void WireguardManagerBridge::deleteProfile(const QString &name)
{
    m_proxy->DeleteProfile(name);
}
