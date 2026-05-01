#include "WireguardManager.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QNetworkInterface>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>

#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>

static const QRegularExpression s_nameRegex(ProfileNamePattern);

WireguardManager::WireguardManager(QObject *parent)
    : QObject(parent)
{
}

ProfileList WireguardManager::ListProfiles()
{
    ProfileList profiles;
    QDir dir(QStringLiteral("/etc/wireguard"));
    const QStringList configs = dir.entryList({QStringLiteral("*.conf")}, QDir::Files);

    for (const QString &conf : configs) {
        const QString name = QFileInfo(conf).completeBaseName();
        const bool active = QNetworkInterface::interfaceFromName(name).isValid();

        quint64 rx = 0, tx = 0;
        if (active) {
            auto readStat = [&](const QString &stat) -> quint64 {
                QFile f(QStringLiteral("/sys/class/net/%1/statistics/%2").arg(name, stat));
                if (f.open(QIODevice::ReadOnly))
                    return f.readAll().trimmed().toULongLong();
                return 0;
            };
            rx = readStat(QStringLiteral("rx_bytes"));
            tx = readStat(QStringLiteral("tx_bytes"));
        }

        profiles.append({name, active ? QStringLiteral("active") : QStringLiteral("inactive"), rx, tx});
    }
    return profiles;
}

void WireguardManager::ToggleProfile(const QString &name, bool targetState)
{
    if (!s_nameRegex.match(name).hasMatch()) {
        sendErrorReply(QDBusError::InvalidArgs, QStringLiteral("Invalid profile name"));
        return;
    }

    setDelayedReply(true);
    const QDBusMessage msg = message();
    QDBusConnection conn = connection();

    auto *auth = PolkitQt1::Authority::instance();
    const auto subject = PolkitQt1::SystemBusNameSubject(msg.service());

    connect(auth, &PolkitQt1::Authority::checkAuthorizationFinished,
            this, [this, name, targetState, msg, conn](PolkitQt1::Authority::Result result) mutable {

        if (result != PolkitQt1::Authority::Yes) {
            conn.send(msg.createErrorReply(
                QDBusError::AccessDenied,
                QStringLiteral("Polkit: access denied")));
            return;
        }

        conn.send(msg.createReply());

        QProcess *proc = new QProcess(this);
        const QString cmd = targetState ? QStringLiteral("up") : QStringLiteral("down");
        proc->setProgram(QStringLiteral("/usr/bin/wg-quick"));
        proc->setArguments({cmd, QStringLiteral("/etc/wireguard/%1.conf").arg(name)});

        connect(proc, &QProcess::errorOccurred,
                this, [this, name, proc](QProcess::ProcessError) {
            qWarning() << "wg-quick failed to start:" << proc->errorString();
            emit ErrorOccurred(name, QStringLiteral("wg-quick failed to start: %1")
                               .arg(proc->errorString()));
            proc->deleteLater();
        });

        connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, name, targetState, proc](int exitCode, QProcess::ExitStatus) {
            const QString err = QString::fromLocal8Bit(proc->readAllStandardError());
            const QString out = QString::fromLocal8Bit(proc->readAllStandardOutput());
            if (exitCode != 0) {
                qWarning() << "wg-quick failed (exit" << exitCode << "):" << err;
                emit ErrorOccurred(name, err.trimmed());
            } else {
                qInfo() << "wg-quick succeeded:" << out.trimmed();
                emit ProfileStatusChanged(name,
                    targetState ? QStringLiteral("active") : QStringLiteral("inactive"));
            }
            proc->deleteLater();
        });

        qInfo() << "Executing wg-quick" << cmd << "for profile" << name;
        proc->start();

    }, Qt::SingleShotConnection);

    auth->checkAuthorization(
        QStringLiteral("io.github.traciges.wireguard.toggle"),
        subject,
        PolkitQt1::Authority::AllowUserInteraction);
}

void WireguardManager::ImportProfile(const QString &name, const QString &contents)
{
    if (!s_nameRegex.match(name).hasMatch()) {
        sendErrorReply(QDBusError::InvalidArgs, QStringLiteral("Invalid profile name"));
        return;
    }

    if (contents.toUtf8().size() > MaxConfigSizeBytes) {
        sendErrorReply(QDBusError::InvalidArgs, QStringLiteral("Configuration file too large"));
        return;
    }

    const QString destPath = QStringLiteral("/etc/wireguard/%1.conf").arg(name);
    if (QFileInfo::exists(destPath)) {
        sendErrorReply(QDBusError::InvalidArgs,
                       QStringLiteral("Profile \"%1\" already exists").arg(name));
        return;
    }

    setDelayedReply(true);
    const QDBusMessage msg = message();
    QDBusConnection conn = connection();

    auto *auth = PolkitQt1::Authority::instance();
    const auto subject = PolkitQt1::SystemBusNameSubject(msg.service());

    connect(auth, &PolkitQt1::Authority::checkAuthorizationFinished,
            this, [this, name, contents, destPath, msg, conn](PolkitQt1::Authority::Result result) mutable {

        if (result != PolkitQt1::Authority::Yes) {
            conn.send(msg.createErrorReply(
                QDBusError::AccessDenied,
                QStringLiteral("Polkit: access denied")));
            return;
        }

        QSaveFile file(destPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            conn.send(msg.createErrorReply(
                QDBusError::Failed,
                QStringLiteral("Could not create file: %1").arg(file.errorString())));
            return;
        }

        file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        file.write(contents.toUtf8());

        if (!file.commit()) {
            conn.send(msg.createErrorReply(
                QDBusError::Failed,
                QStringLiteral("Could not save file: %1").arg(file.errorString())));
            return;
        }

        qInfo() << "Imported WireGuard profile:" << name;
        conn.send(msg.createReply());
        emit ProfileImported(name);

    }, Qt::SingleShotConnection);

    auth->checkAuthorization(
        QStringLiteral("io.github.traciges.wireguard.import"),
        subject,
        PolkitQt1::Authority::AllowUserInteraction);
}

QString WireguardManager::ExportProfile(const QString &name)
{
    if (!s_nameRegex.match(name).hasMatch()) {
        sendErrorReply(QDBusError::InvalidArgs, QStringLiteral("Invalid profile name"));
        return {};
    }

    const QString filePath = QStringLiteral("/etc/wireguard/%1.conf").arg(name);
    if (!QFileInfo::exists(filePath)) {
        sendErrorReply(QDBusError::InvalidArgs,
                       QStringLiteral("Profile \"%1\" not found").arg(name));
        return {};
    }

    setDelayedReply(true);
    const QDBusMessage msg = message();
    QDBusConnection conn = connection();

    auto *auth = PolkitQt1::Authority::instance();
    const auto subject = PolkitQt1::SystemBusNameSubject(msg.service());

    connect(auth, &PolkitQt1::Authority::checkAuthorizationFinished,
            this, [filePath, msg, conn](PolkitQt1::Authority::Result result) mutable {

        if (result != PolkitQt1::Authority::Yes) {
            conn.send(msg.createErrorReply(
                QDBusError::AccessDenied,
                QStringLiteral("Polkit: access denied")));
            return;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            conn.send(msg.createErrorReply(
                QDBusError::Failed,
                QStringLiteral("Could not read file: %1").arg(file.errorString())));
            return;
        }

        const QString contents = QString::fromUtf8(file.readAll());
        conn.send(msg.createReply(contents));

    }, Qt::SingleShotConnection);

    auth->checkAuthorization(
        QStringLiteral("io.github.traciges.wireguard.export"),
        subject,
        PolkitQt1::Authority::AllowUserInteraction);

    return {};
}

void WireguardManager::DeleteProfile(const QString &name)
{
    if (!s_nameRegex.match(name).hasMatch()) {
        sendErrorReply(QDBusError::InvalidArgs, QStringLiteral("Invalid profile name"));
        return;
    }

    if (QNetworkInterface::interfaceFromName(name).isValid()) {
        sendErrorReply(QDBusError::InvalidArgs,
                       QStringLiteral("Profile \"%1\" is active - please disconnect first").arg(name));
        return;
    }

    const QString filePath = QStringLiteral("/etc/wireguard/%1.conf").arg(name);
    if (!QFileInfo::exists(filePath)) {
        sendErrorReply(QDBusError::InvalidArgs,
                       QStringLiteral("Profile \"%1\" not found").arg(name));
        return;
    }

    setDelayedReply(true);
    const QDBusMessage msg = message();
    QDBusConnection conn = connection();

    auto *auth = PolkitQt1::Authority::instance();
    const auto subject = PolkitQt1::SystemBusNameSubject(msg.service());

    connect(auth, &PolkitQt1::Authority::checkAuthorizationFinished,
            this, [this, name, filePath, msg, conn](PolkitQt1::Authority::Result result) mutable {

        if (result != PolkitQt1::Authority::Yes) {
            conn.send(msg.createErrorReply(
                QDBusError::AccessDenied,
                QStringLiteral("Polkit: access denied")));
            return;
        }

        if (!QFile::remove(filePath)) {
            conn.send(msg.createErrorReply(
                QDBusError::Failed,
                QStringLiteral("Could not delete file")));
            return;
        }

        qInfo() << "Deleted WireGuard profile:" << name;
        conn.send(msg.createReply());
        emit ProfileDeleted(name);

    }, Qt::SingleShotConnection);

    auth->checkAuthorization(
        QStringLiteral("io.github.traciges.wireguard.delete"),
        subject,
        PolkitQt1::Authority::AllowUserInteraction);
}

void WireguardManager::RenameProfile(const QString &oldName, const QString &newName)
{
    if (!s_nameRegex.match(oldName).hasMatch()) {
        sendErrorReply(QDBusError::InvalidArgs, QStringLiteral("Invalid source profile name"));
        return;
    }
    if (!s_nameRegex.match(newName).hasMatch()) {
        sendErrorReply(QDBusError::InvalidArgs, QStringLiteral("Invalid new profile name"));
        return;
    }
    if (oldName == newName) {
        sendErrorReply(QDBusError::InvalidArgs, QStringLiteral("Names are identical"));
        return;
    }
    if (QNetworkInterface::interfaceFromName(oldName).isValid()) {
        sendErrorReply(QDBusError::InvalidArgs,
                       QStringLiteral("Profile \"%1\" is active - please disconnect first").arg(oldName));
        return;
    }
    const QString srcPath = QStringLiteral("/etc/wireguard/%1.conf").arg(oldName);
    if (!QFileInfo::exists(srcPath)) {
        sendErrorReply(QDBusError::InvalidArgs,
                       QStringLiteral("Profile \"%1\" not found").arg(oldName));
        return;
    }
    const QString dstPath = QStringLiteral("/etc/wireguard/%1.conf").arg(newName);
    if (QFileInfo::exists(dstPath)) {
        sendErrorReply(QDBusError::InvalidArgs,
                       QStringLiteral("Profile \"%1\" already exists").arg(newName));
        return;
    }

    setDelayedReply(true);
    const QDBusMessage msg = message();
    QDBusConnection conn = connection();

    auto *auth = PolkitQt1::Authority::instance();
    const auto subject = PolkitQt1::SystemBusNameSubject(msg.service());

    connect(auth, &PolkitQt1::Authority::checkAuthorizationFinished,
            this, [this, oldName, newName, srcPath, dstPath, msg, conn]
                  (PolkitQt1::Authority::Result result) mutable {

        if (result != PolkitQt1::Authority::Yes) {
            conn.send(msg.createErrorReply(QDBusError::AccessDenied,
                                           QStringLiteral("Polkit: access denied")));
            return;
        }
        if (!QFile::rename(srcPath, dstPath)) {
            conn.send(msg.createErrorReply(QDBusError::Failed,
                                           QStringLiteral("Could not rename file")));
            return;
        }
        qInfo() << "Renamed WireGuard profile:" << oldName << "->" << newName;
        conn.send(msg.createReply());
        emit ProfileRenamed(oldName, newName);

    }, Qt::SingleShotConnection);

    auth->checkAuthorization(
        QStringLiteral("io.github.traciges.wireguard.rename"),
        subject,
        PolkitQt1::Authority::AllowUserInteraction);
}
