#include "WireguardManager.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDir>
#include <QFileInfo>
#include <QNetworkInterface>
#include <QProcess>
#include <QRegularExpression>

#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>

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
        profiles.append({name, active ? QStringLiteral("active") : QStringLiteral("inactive")});
    }
    return profiles;
}

void WireguardManager::ToggleProfile(const QString &name, bool targetState)
{
    // Path Traversal Guard – WireGuard interface names: max 15 chars, IFNAMSIZ limit
    static const QRegularExpression nameRegex(QStringLiteral("^[a-zA-Z0-9_=+.-]{1,15}$"));
    if (!nameRegex.match(name).hasMatch()) {
        sendErrorReply(QDBusError::InvalidArgs, QStringLiteral("Ungültiger Profilname"));
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
                QStringLiteral("Polkit: Zugriff verweigert")));
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
            emit ErrorOccurred(name, QStringLiteral("wg-quick konnte nicht gestartet werden: %1")
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
