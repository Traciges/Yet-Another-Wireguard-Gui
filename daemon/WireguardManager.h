#pragma once
#include <QDBusContext>
#include <QObject>
#include "WireguardTypes.h"

class WireguardManager : public QObject, protected QDBusContext {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.github.traciges.WireguardManager")

public:
    explicit WireguardManager(QObject *parent = nullptr);

public Q_SLOTS:
    ProfileList ListProfiles();
    void ToggleProfile(const QString &name, bool targetState);
    void ImportProfile(const QString &name, const QString &contents);
    void DeleteProfile(const QString &name);
    QString ExportProfile(const QString &name);

Q_SIGNALS:
    void ProfileStatusChanged(const QString &name, const QString &status);
    void ProfileImported(const QString &name);
    void ProfileDeleted(const QString &name);
    void ErrorOccurred(const QString &profileName, const QString &errorMessage);
};
