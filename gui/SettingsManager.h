#pragma once
#include <QObject>
#include <QSettings>
#include <QString>

class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool autostartEnabled READ autostartEnabled NOTIFY autostartEnabledChanged)
    Q_PROPERTY(QString autoConnectProfile READ autoConnectProfile NOTIFY autoConnectProfileChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);

    bool autostartEnabled() const;
    QString autoConnectProfile() const;

public slots:
    void setAutostartEnabled(bool enabled);
    void setAutoConnectProfile(const QString &profile);

signals:
    void autostartEnabledChanged();
    void autoConnectProfileChanged();

private:
    QSettings m_settings;
    QString autostartDesktopPath() const;
    bool writeAutostartFile();
    void removeAutostartFile();
};
