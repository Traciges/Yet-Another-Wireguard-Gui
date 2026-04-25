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

Q_SIGNALS:
    void ProfileStatusChanged(const QString &name, const QString &status);
    void ErrorOccurred(const QString &profileName, const QString &errorMessage);
};
