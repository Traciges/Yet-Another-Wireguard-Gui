#pragma once
#include <QDBusArgument>
#include <QLatin1String>
#include <QList>
#include <QMetaType>
#include <QString>

// WireGuard interface name constraint (IFNAMSIZ = 15 chars max)
inline constexpr QLatin1String ProfileNamePattern{"^[a-zA-Z0-9_=+.\\-]{1,15}$"};
inline constexpr qint64 MaxConfigSizeBytes = 65536;
inline constexpr QLatin1String DBusServiceName{"io.github.traciges.WireguardManager"};
inline constexpr QLatin1String DBusObjectPath{"/io/github/traciges/WireguardManager"};

struct ProfileInfo {
    QString name;
    QString status;
    quint64 rxBytes = 0;
    quint64 txBytes = 0;
};
Q_DECLARE_METATYPE(ProfileInfo)

typedef QList<ProfileInfo> ProfileList;
Q_DECLARE_METATYPE(ProfileList)

inline QDBusArgument &operator<<(QDBusArgument &arg, const ProfileInfo &info)
{
    arg.beginStructure();
    arg << info.name << info.status << info.rxBytes << info.txBytes;
    arg.endStructure();
    return arg;
}

inline const QDBusArgument &operator>>(const QDBusArgument &arg, ProfileInfo &info)
{
    arg.beginStructure();
    arg >> info.name >> info.status >> info.rxBytes >> info.txBytes;
    arg.endStructure();
    return arg;
}
