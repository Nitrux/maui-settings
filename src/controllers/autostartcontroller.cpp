#include "autostartcontroller.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>

namespace
{
struct ManagedServices
{
    QStringList names;
    QStringList commands;
    bool found = false;
    bool valid = true;
};

QString configLocation()
{
    QString location = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (location.isEmpty())
        location = QDir::homePath() + QStringLiteral("/.config");
    return location;
}

QString autostartServiceDirectory()
{
    return configLocation() + QStringLiteral("/rc/init.d");
}

QString autostartRunnerDirectory()
{
    return configLocation() + QStringLiteral("/maui-settings/autostart");
}

QString shellQuote(const QString &value)
{
    const QString quote(QChar(39));
    const QString escapedQuote = quote + QString(QChar(92)) + quote + quote;
    QString result = value;
    result.replace(quote, escapedQuote);
    return quote + result + quote;
}

QString newServiceName()
{
    return QStringLiteral("maui-autostart-")
        + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool writeOwnedFile(const QString &path, const QByteArray &content, QFileDevice::Permissions permissions)
{
    QSaveFile destination(path);
    if (!destination.open(QIODevice::WriteOnly))
        return false;

    if (destination.write(content) != content.size() || !destination.commit())
        return false;

    return QFile::setPermissions(path, permissions);
}

ManagedServices readManagedServices(const QString &directory)
{
    ManagedServices result;
    const QDir serviceDirectory(directory);
    const QFileInfoList files = serviceDirectory.entryInfoList(
        {QStringLiteral("maui-autostart-*")},
        QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
        QDir::Name);

    for (const QFileInfo &fileInfo : files)
    {
        result.found = true;

        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            result.valid = false;
            continue;
        }

        const QByteArray data = file.readAll();
        const QByteArray headerPrefix = QByteArrayLiteral("# Command-Base64: ");
        QByteArray header;
        for (const QByteArray &line : data.split(static_cast<char>(10)))
        {
            if (line.startsWith(headerPrefix)) {
                header = line;
                break;
            }
        }
        if (header.isEmpty()) {
            result.valid = false;
            continue;
        }

        const QByteArray encoded = header.mid(headerPrefix.size()).trimmed();
        const QByteArray decoded = QByteArray::fromBase64(encoded);
        const QString command = QString::fromUtf8(decoded);
        if (command.isEmpty() || command.contains(QChar(10)) || command.contains(QChar(13))) {
            result.valid = false;
            continue;
        }

        result.names.append(fileInfo.fileName());
        result.commands.append(command);
    }

    return result;
}
} // namespace

AutostartController::AutostartController(QObject *parent)
    : QObject(parent)
    , m_configPath(autostartServiceDirectory())
    , m_available(!QStandardPaths::findExecutable(QStringLiteral("nwsm")).isEmpty()
        && !QStandardPaths::findExecutable(QStringLiteral("rc-update")).isEmpty()
        && QDir().mkpath(m_configPath))
{
    reload();
}

QString AutostartController::configPath() const
{
    return m_configPath;
}

bool AutostartController::available() const
{
    return m_available;
}

QStringList AutostartController::commands() const
{
    return m_commands;
}

QString AutostartController::errorMessage() const
{
    return m_errorMessage;
}

void AutostartController::reload()
{
    if (!m_available) {
        setErrorMessage({});
        return;
    }

    const ManagedServices managed = readManagedServices(m_configPath);
    if (!managed.valid) {
        setErrorMessage(tr("One or more Maui Settings autostart services are invalid."));
        return;
    }

    QStringList commands;
    QStringList names;
    if (managed.found) {
        commands = managed.commands;
        names = managed.names;
    }

    const bool changed = commands != m_commands;
    m_commands = commands;
    m_serviceNames = names;
    if (changed)
        Q_EMIT commandsChanged();

    setErrorMessage({});
}

bool AutostartController::save()
{
    if (!m_available) {
        setErrorMessage(tr("OpenRC user-service management is unavailable."));
        return false;
    }

    const ManagedServices previous = readManagedServices(m_configPath);

    while (m_serviceNames.size() < m_commands.size())
        m_serviceNames.append(newServiceName());
    while (m_serviceNames.size() > m_commands.size())
        m_serviceNames.removeLast();

    const QString runnerDirectory = autostartRunnerDirectory();
    if (!QDir().mkpath(runnerDirectory)) {
        setErrorMessage(tr("Could not create the Maui Settings autostart directory."));
        return false;
    }

    const QFileDevice::Permissions permissions = QFileDevice::ReadOwner
        | QFileDevice::WriteOwner
        | QFileDevice::ExeOwner;

    for (int index = 0; index < m_commands.size(); ++index)
    {
        const QString &serviceName = m_serviceNames.at(index);
        const QString runnerPath = runnerDirectory + QLatin1Char('/') + serviceName;
        const QString runner = QStringLiteral("#!/bin/sh\nexec /bin/sh -c %1\n")
            .arg(shellQuote(m_commands.at(index)));

        if (!writeOwnedFile(runnerPath, runner.toUtf8(), permissions)) {
            setErrorMessage(tr("Could not write the Maui Settings autostart runner."));
            return false;
        }

        const QString servicePath = m_configPath + QLatin1Char('/') + serviceName;
        const QString service = QStringLiteral(
            "#!/sbin/openrc-run\n"
            "# Managed by Maui Settings.\n"
            "# Command-Base64: %1\n"
            "command=%2\n"
            "command_background=true\n"
            "pidfile=\"${XDG_RUNTIME_DIR}/maui-settings/${RC_SVCNAME}.pid\"\n"
            "start_pre() {\n"
            "    mkdir -p \"${XDG_RUNTIME_DIR}/maui-settings\"\n"
            "}\n")
            .arg(QString::fromLatin1(m_commands.at(index).toUtf8().toBase64()),
                shellQuote(runnerPath));

        if (!writeOwnedFile(servicePath, service.toUtf8(), permissions)) {
            setErrorMessage(tr("Could not write the Maui Settings OpenRC service."));
            return false;
        }
    }

    const QString rcUpdate = QStandardPaths::findExecutable(QStringLiteral("rc-update"));
    for (const QString &name : previous.names)
    {
        if (!m_serviceNames.contains(name))
        {
            QProcess::execute(rcUpdate, {QStringLiteral("-U"), QStringLiteral("delete"),
                name, QStringLiteral("desktop")});
            QFile::remove(m_configPath + QLatin1Char('/') + name);
            QFile::remove(runnerDirectory + QLatin1Char('/') + name);
        }
    }

    const QString nwsm = QStandardPaths::findExecutable(QStringLiteral("nwsm"));
    if (QProcess::execute(nwsm, {QStringLiteral("reconcile")}) != 0) {
        setErrorMessage(tr("Autostart was saved, but NWSM could not apply it until the next session."));
        return true;
    }

    setErrorMessage({});
    return true;
}

bool AutostartController::validCommand(const QString &command) const
{
    const QString normalized = command.trimmed();
    return !normalized.isEmpty()
        && !normalized.contains(QChar(10))
        && !normalized.contains(QChar(13));
}

bool AutostartController::addCommand(const QString &command)
{
    if (!validCommand(command)) {
        setErrorMessage(tr("Enter an autostart command."));
        return false;
    }

    const QStringList previousCommands = m_commands;
    const QStringList previousNames = m_serviceNames;
    m_commands.append(command.trimmed());
    m_serviceNames.append(newServiceName());

    if (!save()) {
        m_commands = previousCommands;
        m_serviceNames = previousNames;
        Q_EMIT commandsChanged();
        return false;
    }

    Q_EMIT commandsChanged();
    return true;
}

bool AutostartController::updateCommand(int index, const QString &command)
{
    if (index < 0 || index >= m_commands.size()) {
        setErrorMessage(tr("The selected autostart command is no longer available."));
        return false;
    }

    if (!validCommand(command)) {
        setErrorMessage(tr("Enter an autostart command."));
        return false;
    }

    const QString normalized = command.trimmed();
    if (m_commands.at(index) == normalized)
        return true;

    const QStringList previousCommands = m_commands;
    const QStringList previousNames = m_serviceNames;
    m_commands[index] = normalized;

    if (!save()) {
        m_commands = previousCommands;
        m_serviceNames = previousNames;
        Q_EMIT commandsChanged();
        return false;
    }

    Q_EMIT commandsChanged();
    return true;
}

bool AutostartController::removeCommand(int index)
{
    if (index < 0 || index >= m_commands.size()) {
        setErrorMessage(tr("The selected autostart command is no longer available."));
        return false;
    }

    const QStringList previousCommands = m_commands;
    const QStringList previousNames = m_serviceNames;
    m_commands.removeAt(index);
    if (index < m_serviceNames.size())
        m_serviceNames.removeAt(index);

    if (!save()) {
        m_commands = previousCommands;
        m_serviceNames = previousNames;
        Q_EMIT commandsChanged();
        return false;
    }

    Q_EMIT commandsChanged();
    return true;
}

void AutostartController::clearError()
{
    setErrorMessage({});
}

void AutostartController::setErrorMessage(const QString &message)
{
    if (m_errorMessage == message)
        return;

    m_errorMessage = message;
    Q_EMIT errorMessageChanged();
}
