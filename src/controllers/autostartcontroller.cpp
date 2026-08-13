#include "autostartcontroller.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QProcess>

namespace
{
struct AutostartRange
{
    int start = -1;
    int end = -1;
};

QString autostartConfigPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + QStringLiteral("/hypr/hyprland.lua");
}

AutostartRange findAutostartBlock(const QStringList &lines)
{
    const QRegularExpression startExpression(
        QStringLiteral("^\\s*hl\\.on\\(\\s*\"hyprland\\.start\"\\s*,\\s*function\\s*\\(\\s*\\)\\s*$"));
    const QRegularExpression endExpression(QStringLiteral("^\\s*end\\s*\\)\\s*$"));

    for (int start = 0; start < lines.size(); ++start)
    {
        if (!startExpression.match(lines.at(start)).hasMatch())
            continue;

        for (int end = start + 1; end < lines.size(); ++end)
        {
            if (endExpression.match(lines.at(end)).hasMatch())
                return {start, end};
        }
    }

    return {};
}

QString decodeLuaString(const QString &value)
{
    QString result;
    result.reserve(value.size());

    bool escaped = false;
    for (const QChar character : value)
    {
        if (escaped)
        {
            if (character == QChar(34) || character == QChar(92))
                result.append(character);
            else
            {
                result.append(QChar(92));
                result.append(character);
            }
            escaped = false;
        }
        else if (character == QChar(92))
            escaped = true;
        else
            result.append(character);
    }

    if (escaped)
        result.append(QChar(92));

    return result;
}

QString encodeLuaString(const QString &value)
{
    QString result = value;
    result.replace(QChar(92), QStringLiteral("\\\\"));
    result.replace(QChar(34), QStringLiteral("\\\""));
    return result;
}

QRegularExpression execExpression()
{
    return QRegularExpression(
        QStringLiteral("^\\s*hl\\.exec_cmd\\(\\s*\"((?:\\\\.|[^\"\\\\])*)\"\\s*\\).*"));
}

QStringList commandLines(const QStringList &lines, const AutostartRange range, QList<int> *lineNumbers)
{
    QStringList commands;
    const QRegularExpression expression = execExpression();

    if (lineNumbers)
        lineNumbers->clear();

    if (range.start < 0 || range.end <= range.start)
        return commands;

    for (int line = range.start + 1; line < range.end; ++line)
    {
        const auto match = expression.match(lines.at(line));
        if (!match.hasMatch())
            continue;

        commands.append(decodeLuaString(match.captured(1)));
        if (lineNumbers)
            lineNumbers->append(line);
    }

    return commands;
}

bool replaceAutostartBlock(QStringList &lines, const QStringList &commands)
{
    AutostartRange range = findAutostartBlock(lines);
    if (range.start < 0)
    {
        if (!lines.isEmpty() && !lines.constLast().isEmpty())
            lines.append(QString());

        lines.append(QStringLiteral("hl.on(\"hyprland.start\", function()"));
        for (const QString &command : commands)
            lines.append(QStringLiteral("    hl.exec_cmd(\"%1\")").arg(encodeLuaString(command)));
        lines.append(QStringLiteral("end)"));
        return true;
    }

    QList<int> lineNumbers;
    commandLines(lines, range, &lineNumbers);

    const int insertionLine = lineNumbers.isEmpty() ? range.end : lineNumbers.first();
    for (auto line = lineNumbers.crbegin(); line != lineNumbers.crend(); ++line)
        lines.removeAt(*line);

    for (int i = 0; i < commands.size(); ++i)
        lines.insert(insertionLine + i, QStringLiteral("    hl.exec_cmd(\"%1\")").arg(encodeLuaString(commands.at(i))));

    return true;
}
} // namespace

AutostartController::AutostartController(QObject *parent)
    : QObject(parent)
    , m_configPath(autostartConfigPath())
    , m_available(QFileInfo::exists(m_configPath))
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
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setErrorMessage(tr("Could not open %1: %2").arg(m_configPath, file.errorString()));
        return;
    }

    const QStringList lines = QString::fromUtf8(file.readAll()).split(QChar(10));
    const QStringList commands = commandLines(lines, findAutostartBlock(lines), nullptr);
    if (commands != m_commands)
    {
        m_commands = commands;
        Q_EMIT commandsChanged();
    }

    setErrorMessage({});
}

bool AutostartController::save()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setErrorMessage(tr("Could not open %1: %2").arg(m_configPath, file.errorString()));
        return false;
    }

    QStringList lines = QString::fromUtf8(file.readAll()).split(QChar(10));
    if (!replaceAutostartBlock(lines, m_commands))
    {
        setErrorMessage(tr("Could not locate the Hyprland autostart block."));
        return false;
    }

    QSaveFile destination(m_configPath);
    if (!destination.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        setErrorMessage(tr("Could not write %1: %2").arg(m_configPath, destination.errorString()));
        return false;
    }

    QTextStream stream(&destination);
    stream << lines.join(QChar(10));
    stream.flush();
    if (!destination.commit())
    {
        setErrorMessage(tr("Could not replace %1: %2").arg(m_configPath, destination.errorString()));
        return false;
    }

    const QString hyprctl = QStandardPaths::findExecutable(QStringLiteral("hyprctl"));
    if (!hyprctl.isEmpty())
        QProcess::startDetached(hyprctl, {QStringLiteral("reload")});

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
    if (!validCommand(command))
    {
        setErrorMessage(tr("Enter an autostart command."));
        return false;
    }

    m_commands.append(command.trimmed());
    Q_EMIT commandsChanged();
    setErrorMessage({});
    return true;
}

bool AutostartController::updateCommand(int index, const QString &command)
{
    if (index < 0 || index >= m_commands.size())
    {
        setErrorMessage(tr("The selected autostart command is no longer available."));
        return false;
    }

    if (!validCommand(command))
    {
        setErrorMessage(tr("Enter an autostart command."));
        return false;
    }

    const QString normalized = command.trimmed();
    if (m_commands.at(index) == normalized)
        return true;

    m_commands[index] = normalized;
    Q_EMIT commandsChanged();
    setErrorMessage({});
    return true;
}

bool AutostartController::removeCommand(int index)
{
    if (index < 0 || index >= m_commands.size())
    {
        setErrorMessage(tr("The selected autostart command is no longer available."));
        return false;
    }

    m_commands.removeAt(index);
    Q_EMIT commandsChanged();
    setErrorMessage({});
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
