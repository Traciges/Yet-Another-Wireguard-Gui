#include "WireguardManager.h"

WireguardManager::WireguardManager(QObject *parent)
    : QObject(parent)
{
}

ProfileList WireguardManager::ListProfiles()
{
    // Hardcoded stub — Milestone 2 will scan /etc/wireguard/*.conf
    return ProfileList{{"Yannik", "inactive"}};
}

void WireguardManager::ToggleProfile(const QString &name, bool targetState)
{
    Q_UNUSED(targetState)
    // Hardcoded stub — Milestone 2 will call wg-quick via Polkit-authorized helper
    emit ErrorOccurred(name, QStringLiteral("Not implemented yet"));
}
