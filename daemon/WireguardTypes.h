#pragma once
#include <QDBusArgument>
#include <QList>
#include <QMetaType>
#include <QString>

struct ProfileInfo {
    QString name;
    QString status;
};
Q_DECLARE_METATYPE(ProfileInfo)

typedef QList<ProfileInfo> ProfileList;
Q_DECLARE_METATYPE(ProfileList)

inline QDBusArgument &operator<<(QDBusArgument &arg, const ProfileInfo &info)
{
    arg.beginStructure();
    arg << info.name << info.status;
    arg.endStructure();
    return arg;
}

inline const QDBusArgument &operator>>(const QDBusArgument &arg, ProfileInfo &info)
{
    arg.beginStructure();
    arg >> info.name >> info.status;
    arg.endStructure();
    return arg;
}
