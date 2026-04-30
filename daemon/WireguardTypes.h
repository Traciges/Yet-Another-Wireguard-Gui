#pragma once
#include <QDBusArgument>
#include <QList>
#include <QMetaType>
#include <QString>

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
