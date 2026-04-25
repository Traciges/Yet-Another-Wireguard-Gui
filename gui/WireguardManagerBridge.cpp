#include "WireguardManagerBridge.h"
#include "WireguardTypes.h"
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QVariantMap>

WireguardManagerBridge::WireguardManagerBridge(
    IoGithubTracigesWireguardManagerInterface *proxy, QObject *parent)
    : QObject(parent), m_proxy(proxy)
{
    connect(m_proxy, &IoGithubTracigesWireguardManagerInterface::ProfileStatusChanged,
            this, &WireguardManagerBridge::profileStatusChanged);
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
