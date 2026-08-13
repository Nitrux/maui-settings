#include "environmentcontroller.h"

#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QVariantMap>

namespace
{
QString environmentConfigPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + QStringLiteral("/hypr/hyprland.lua");
}

QRegularExpression environmentExpression()
{
    return QRegularExpression(
        QStringLiteral("^\\s*hl\\.env\\(\\s*\"([A-Za-z_][A-Za-z0-9_]*)\"\\s*,\\s*\"((?:\\\\.|[^\"\\\\])*)\"\\s*\\)\\s*(?:--.*)?$"));
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

QVariantList parseVariables(const QStringList &lines, QList<int> *lineNumbers)
{
    QVariantList variables;
    const QRegularExpression expression = environmentExpression();

    if (lineNumbers)
        lineNumbers->clear();

    for (int line = 0; line < lines.size(); ++line)
    {
        const auto match = expression.match(lines.at(line));
        if (!match.hasMatch())
            continue;

        QVariantMap variable;
        variable.insert(QStringLiteral("variable"), match.captured(1));
        variable.insert(QStringLiteral("value"), decodeLuaString(match.captured(2)));
        variables.append(variable);

        if (lineNumbers)
            lineNumbers->append(line);
    }

    return variables;
}

int lastEnvironmentLine(const QStringList &lines)
{
    const QRegularExpression expression(
        QStringLiteral("^\\s*(?:--\\s*)?hl\\.env\\("));
    int last = -1;
    for (int line = 0; line < lines.size(); ++line)
    {
        if (expression.match(lines.at(line)).hasMatch())
            last = line;
    }

    return last;
}

bool replaceEnvironmentLines(QStringList &lines, const QVariantList &variables)
{
    QList<int> lineNumbers;
    parseVariables(lines, &lineNumbers);

    int insertionLine = -1;
    if (!lineNumbers.isEmpty())
        insertionLine = lineNumbers.first();
    else
    {
        const int lastLine = lastEnvironmentLine(lines);
        insertionLine = lastLine >= 0 ? lastLine + 1 : lines.size();

        if (lastLine < 0 && !lines.isEmpty() && !lines.constLast().isEmpty())
            lines.append(QString());
    }

    for (auto line = lineNumbers.crbegin(); line != lineNumbers.crend(); ++line)
        lines.removeAt(*line);

    for (int i = 0; i < variables.size(); ++i)
    {
        const QVariantMap variable = variables.at(i).toMap();
        const QString name = variable.value(QStringLiteral("variable")).toString();
        const QString value = variable.value(QStringLiteral("value")).toString();
        lines.insert(insertionLine + i,
                     QStringLiteral("hl.env(\"%1\", \"%2\")")
                         .arg(name, encodeLuaString(value)));
    }

    return true;
}
} // namespace

EnvironmentController::EnvironmentController(QObject *parent)
    : QObject(parent)
    , m_configPath(environmentConfigPath())
    , m_available(QFileInfo::exists(m_configPath))
{
    reload();
}

QString EnvironmentController::configPath() const
{
    return m_configPath;
}

bool EnvironmentController::available() const
{
    return m_available;
}

QVariantList EnvironmentController::variables() const
{
    return m_variables;
}

QString EnvironmentController::errorMessage() const
{
    return m_errorMessage;
}

void EnvironmentController::reload()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setErrorMessage(tr("Could not open %1: %2").arg(m_configPath, file.errorString()));
        return;
    }

    const QVariantList variables = parseVariables(
        QString::fromUtf8(file.readAll()).split(QChar(10)), nullptr);
    if (variables != m_variables)
    {
        m_variables = variables;
        Q_EMIT variablesChanged();
    }

    setErrorMessage({});
}

bool EnvironmentController::save()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setErrorMessage(tr("Could not open %1: %2").arg(m_configPath, file.errorString()));
        return false;
    }

    QStringList lines = QString::fromUtf8(file.readAll()).split(QChar(10));
    if (!replaceEnvironmentLines(lines, m_variables))
    {
        setErrorMessage(tr("Could not update the Hyprland environment block."));
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

bool EnvironmentController::validVariable(const QString &variable, const QString &value, int ignoredIndex) const
{
    const QString normalizedVariable = variable.trimmed();
    const QRegularExpression expression(
        QStringLiteral("^[A-Za-z_][A-Za-z0-9_]*$"));

    if (!expression.match(normalizedVariable).hasMatch()
        || value.contains(QChar(10))
        || value.contains(QChar(13)))
    {
        return false;
    }

    for (int i = 0; i < m_variables.size(); ++i)
    {
        if (i == ignoredIndex)
            continue;

        const QVariantMap existing = m_variables.at(i).toMap();
        if (existing.value(QStringLiteral("variable")).toString() == normalizedVariable)
            return false;
    }

    return true;
}

bool EnvironmentController::addVariable(const QString &variable, const QString &value)
{
    const QString normalizedVariable = variable.trimmed();
    const QString normalizedValue = value.trimmed();

    if (!validVariable(normalizedVariable, normalizedValue))
    {
        setErrorMessage(tr("Enter a unique variable name and a single-line value."));
        return false;
    }

    QVariantMap entry;
    entry.insert(QStringLiteral("variable"), normalizedVariable);
    entry.insert(QStringLiteral("value"), normalizedValue);
    m_variables.append(entry);
    Q_EMIT variablesChanged();
    setErrorMessage({});
    return true;
}

bool EnvironmentController::updateVariable(int index, const QString &variable, const QString &value)
{
    if (index < 0 || index >= m_variables.size())
    {
        setErrorMessage(tr("The selected environment variable is no longer available."));
        return false;
    }

    const QString normalizedVariable = variable.trimmed();
    const QString normalizedValue = value.trimmed();
    if (!validVariable(normalizedVariable, normalizedValue, index))
    {
        setErrorMessage(tr("Enter a unique variable name and a single-line value."));
        return false;
    }

    QVariantMap entry;
    entry.insert(QStringLiteral("variable"), normalizedVariable);
    entry.insert(QStringLiteral("value"), normalizedValue);
    m_variables[index] = entry;
    Q_EMIT variablesChanged();
    setErrorMessage({});
    return true;
}

bool EnvironmentController::removeVariable(int index)
{
    if (index < 0 || index >= m_variables.size())
    {
        setErrorMessage(tr("The selected environment variable is no longer available."));
        return false;
    }

    m_variables.removeAt(index);
    Q_EMIT variablesChanged();
    setErrorMessage({});
    return true;
}

void EnvironmentController::clearError()
{
    setErrorMessage({});
}

void EnvironmentController::setErrorMessage(const QString &message)
{
    if (m_errorMessage == message)
        return;

    m_errorMessage = message;
    Q_EMIT errorMessageChanged();
}
