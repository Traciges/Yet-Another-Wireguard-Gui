#include "SettingsManager.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QRegularExpression>

static const QRegularExpression s_profileNameRegex(QStringLiteral("^[a-zA-Z0-9_=+.\\-]{1,15}$"));

SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("yet-another-wireguard-gui"), QStringLiteral("config"))
{
}

bool SettingsManager::autostartEnabled() const
{
    return m_settings.value(QStringLiteral("autostart/enabled"), false).toBool();
}

QString SettingsManager::autoConnectProfile() const
{
    return m_settings.value(QStringLiteral("autostart/autoConnectProfile"), QString()).toString();
}

void SettingsManager::setAutostartEnabled(bool enabled)
{
    if (autostartEnabled() == enabled)
        return;

    if (enabled) {
        if (!writeAutostartFile())
            return;
    } else {
        removeAutostartFile();
        // Clear auto-connect profile when autostart is disabled
        if (!autoConnectProfile().isEmpty()) {
            m_settings.setValue(QStringLiteral("autostart/autoConnectProfile"), QString());
            emit autoConnectProfileChanged();
        }
    }

    m_settings.setValue(QStringLiteral("autostart/enabled"), enabled);
    m_settings.sync();
    emit autostartEnabledChanged();
}

void SettingsManager::setAutoConnectProfile(const QString &profile)
{
    if (autoConnectProfile() == profile)
        return;

    // Empty string means "no auto-connect" — always valid
    if (!profile.isEmpty() && !s_profileNameRegex.match(profile).hasMatch())
        return;

    m_settings.setValue(QStringLiteral("autostart/autoConnectProfile"), profile);
    m_settings.sync();
    emit autoConnectProfileChanged();
}

QString SettingsManager::autostartDesktopPath() const
{
    const QString configHome = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    return configHome + QStringLiteral("/autostart/yawg-gui.desktop");
}

bool SettingsManager::writeAutostartFile()
{
    const QString path = autostartDesktopPath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << "[Desktop Entry]\n"
        << "Type=Application\n"
        << "Name=Yet Another WireGuard Gui\n"
        << "Exec=/usr/local/bin/yawg-gui\n"
        << "Icon=yawg-gui\n"
        << "Hidden=false\n"
        << "X-GNOME-Autostart-enabled=true\n";

    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                        QFileDevice::ReadGroup | QFileDevice::ReadOther);
    return true;
}

void SettingsManager::removeAutostartFile()
{
    QFile::remove(autostartDesktopPath());
}
