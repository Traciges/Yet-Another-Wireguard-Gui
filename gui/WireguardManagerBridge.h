#pragma once
#include <QObject>
#include <QVariantList>
#include "WireguardManagerProxy.h"

class WireguardManagerBridge : public QObject
{
    Q_OBJECT
public:
    explicit WireguardManagerBridge(OrgExampleWireguardManagerInterface *proxy,
                                    QObject *parent = nullptr);

public slots:
    void refreshProfiles();
    void toggleProfile(const QString &name, bool targetState);

signals:
    void profilesLoaded(const QVariantList &profiles);
    void profileStatusChanged(const QString &name, const QString &status);
    void errorOccurred(const QString &profileName, const QString &errorMessage);

private:
    OrgExampleWireguardManagerInterface *m_proxy;
};
