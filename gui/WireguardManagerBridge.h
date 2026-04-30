#pragma once
#include <QObject>
#include <QUrl>
#include <QVariantList>
#include "WireguardManagerProxy.h"

class WireguardManagerBridge : public QObject
{
    Q_OBJECT
public:
    explicit WireguardManagerBridge(IoGithubTracigesWireguardManagerInterface *proxy,
                                    QObject *parent = nullptr);

public slots:
    void refreshProfiles();
    void toggleProfile(const QString &name, bool targetState);
    void importProfile(const QUrl &fileUrl);
    void deleteProfile(const QString &name);

signals:
    void profilesLoaded(const QVariantList &profiles);
    void profileStatusChanged(const QString &name, const QString &status);
    void profileImported(const QString &name);
    void profileDeleted(const QString &name);
    void errorOccurred(const QString &profileName, const QString &errorMessage);

private:
    IoGithubTracigesWireguardManagerInterface *m_proxy;
};
