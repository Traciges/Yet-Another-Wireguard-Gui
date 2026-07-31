#pragma once
#include <QDBusPendingCall>
#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>
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
    void renameProfile(const QString &oldName, const QString &newName);
    void exportProfile(const QString &name, const QUrl &fileUrl);
    void addProfile(const QVariantMap &config);
    void enableAndStartDaemon();

signals:
    void profilesLoaded(const QVariantList &profiles);
    void profileStatusChanged(const QString &name, const QString &status);
    void profileImported(const QString &name);
    void profileDeleted(const QString &name);
    void profileRenamed(const QString &oldName, const QString &newName);
    void profileExported(const QString &name);
    void errorOccurred(const QString &profileName, const QString &errorMessage);
    void daemonUnavailable();

private:
    // Reports a failed D-Bus call to the UI instead of letting it fail silently
    void watchCall(const QDBusPendingCall &call, const QString &profileName);

    IoGithubTracigesWireguardManagerInterface *m_proxy;
    int m_startupRetries = 0;
    bool m_daemonUnavailable = false;
    static constexpr int MaxStartupRetries = 5;
    static constexpr int StartupRetryDelayMs = 2000;
    static constexpr int PolkitCallTimeoutMs = 120000;
};
