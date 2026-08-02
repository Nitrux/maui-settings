#include "systemfilepersistence.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStringList>
#include <QTemporaryFile>

namespace
{
constexpr auto overlayrootChrootPath = "/usr/sbin/overlayroot-chroot";
constexpr auto installPath = "/usr/bin/install";
constexpr int processStartTimeoutMs = 5000;
constexpr int processFinishTimeoutMs = 30000;

bool fail(const QString &message, QString *errorMessage)
{
    qWarning().noquote() << "SystemFilePersistence:" << message;
    if (errorMessage)
        *errorMessage = message;
    return false;
}

QString permissionMode(QFileDevice::Permissions permissions)
{
    int mode = 0;
    if (permissions.testFlag(QFileDevice::ReadOwner))
        mode |= 0400;
    if (permissions.testFlag(QFileDevice::WriteOwner))
        mode |= 0200;
    if (permissions.testFlag(QFileDevice::ExeOwner))
        mode |= 0100;
    if (permissions.testFlag(QFileDevice::ReadGroup))
        mode |= 0040;
    if (permissions.testFlag(QFileDevice::WriteGroup))
        mode |= 0020;
    if (permissions.testFlag(QFileDevice::ExeGroup))
        mode |= 0010;
    if (permissions.testFlag(QFileDevice::ReadOther))
        mode |= 0004;
    if (permissions.testFlag(QFileDevice::WriteOther))
        mode |= 0002;
    if (permissions.testFlag(QFileDevice::ExeOther))
        mode |= 0001;
    return QString::number(mode, 8).rightJustified(4, QLatin1Char('0'));
}
}

bool SystemFilePersistence::persist(const QString &path, QString *errorMessage)
{
    if (errorMessage)
        errorMessage->clear();

    qDebug() << "SystemFilePersistence::persist" << path;

    const QFileInfo sourceInfo(path);
    if (!sourceInfo.isAbsolute() || !sourceInfo.exists() || !sourceInfo.isFile()
        || !sourceInfo.isReadable())
    {
        return fail(QStringLiteral("Cannot persist invalid system file %1.").arg(path), errorMessage);
    }

    QFile source(path);
    if (!source.open(QIODevice::ReadOnly))
        return fail(QStringLiteral("Could not read %1 for persistence.").arg(path), errorMessage);

    // /run is bind-mounted by overlayroot-chroot, unlike the tmpfs upper layer.
    QTemporaryFile stagingFile(QStringLiteral("/run/maui-settings-persistence-XXXXXX"));
    if (!stagingFile.open())
        return fail(QStringLiteral("Could not create the persistence staging file in /run."), errorMessage);

    while (!source.atEnd())
    {
        const QByteArray data = source.read(64 * 1024);
        if (data.isEmpty() && source.error() != QFileDevice::NoError)
            return fail(QStringLiteral("Could not read %1 for persistence.").arg(path), errorMessage);
        if (stagingFile.write(data) != data.size())
            return fail(QStringLiteral("Could not write the persistence staging file."), errorMessage);
    }
    if (!stagingFile.flush())
        return fail(QStringLiteral("Could not flush the persistence staging file."), errorMessage);

    qDebug() << "SystemFilePersistence: staged live file"
             << "sourceSize=" << sourceInfo.size()
             << "stagingPath=" << stagingFile.fileName()
             << "stagingSize=" << stagingFile.size();

    QProcess process;
    process.setProgram(QString::fromLatin1(overlayrootChrootPath));
    const QStringList processArguments = {
        QString::fromLatin1(installPath),
        QStringLiteral("-o"), QString::number(sourceInfo.ownerId()),
        QStringLiteral("-g"), QString::number(sourceInfo.groupId()),
        QStringLiteral("-m"), permissionMode(sourceInfo.permissions()),
        stagingFile.fileName(), path,
    };
    process.setArguments(processArguments);
    qDebug() << "SystemFilePersistence: starting"
             << process.program() << processArguments;
    process.start();

    if (!process.waitForStarted(processStartTimeoutMs))
    {
        return fail(QStringLiteral("Could not start overlayroot-chroot: %1")
            .arg(process.errorString()), errorMessage);
    }
    if (!process.waitForFinished(processFinishTimeoutMs))
    {
        process.kill();
        process.waitForFinished();
        return fail(QStringLiteral("Timed out while persisting %1.").arg(path), errorMessage);
    }
    const QString standardOutput = QString::fromUtf8(
        process.readAllStandardOutput()).trimmed();
    const QString standardError = QString::fromUtf8(
        process.readAllStandardError()).trimmed();
    qDebug().noquote() << "SystemFilePersistence: overlayroot-chroot finished"
                       << "exitStatus=" << process.exitStatus()
                       << "exitCode=" << process.exitCode()
                       << "stdout=" << standardOutput
                       << "stderr=" << standardError;
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
    {
        return fail(standardError.isEmpty()
            ? QStringLiteral("overlayroot-chroot could not persist %1.").arg(path)
            : QStringLiteral("overlayroot-chroot could not persist %1: %2").arg(path, standardError),
            errorMessage);
    }

    qDebug() << "SystemFilePersistence: persistent lower file installed successfully";
    return true;
}
