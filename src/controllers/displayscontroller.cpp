#include "displayscontroller.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QVariantMap>
#include <QtGlobal>

namespace
{
QString defaultConfigPath()
{
    return QDir::homePath() + QStringLiteral("/.config/hyprscreend/hyprscreend.conf");
}

QString defaultHyprConfigPath()
{
    return QDir::homePath() + QStringLiteral("/.config/hypr/hyprland.lua");
}

QString expandHome(QString value)
{
    return value.replace(QStringLiteral("$HOME"), QDir::homePath());
}

QString formatRefreshRate(double value)
{
    return QString::number(value, 'f', value == qRound(value) ? 0 : 2);
}

QString luaEscape(QString value)
{
    value.replace(QStringLiteral("\\"), QStringLiteral("\\\\"));
    value.replace(QStringLiteral("\""), QStringLiteral("\\\""));
    return value;
}

QString blockValue(const QString &block, const QString &key)
{
    const QRegularExpression expression(
        QStringLiteral(R"REGEX((?:^|\n)[ \t]*%1[ \t]*=[ \t]*"([^"]*)")REGEX").arg(QRegularExpression::escape(key)));
    const auto match = expression.match(block);
    return match.hasMatch() ? match.captured(1) : QString();
}
}

DisplaysController::DisplaysController(QObject *parent)
    : QObject(parent)
    , m_configPath(defaultConfigPath())
    , m_available(!QStandardPaths::findExecutable(QStringLiteral("hyprscreend")).isEmpty()
                  && !QStandardPaths::findExecutable(QStringLiteral("hyprctl")).isEmpty())
{
    reload();
}

bool DisplaysController::available() const { return m_available; }
bool DisplaysController::configAvailable() const { return m_configAvailable; }
bool DisplaysController::automatic() const { return m_automatic; }
QVariantList DisplaysController::monitors() const { return m_monitorsVariant; }
int DisplaysController::selectedMonitorIndex() const { return m_selectedMonitorIndex; }

void DisplaysController::setSelectedMonitorIndex(int index)
{
    index = m_monitors.isEmpty() ? 0 : qBound(0, index, m_monitors.size() - 1);
    if (m_selectedMonitorIndex == index)
        return;
    m_selectedMonitorIndex = index;
    Q_EMIT selectedMonitorChanged();
    Q_EMIT selectedSettingsChanged();
}

void DisplaysController::selectMonitor(int index)
{
    setSelectedMonitorIndex(index);
}

QStringList DisplaysController::resolutions() const
{
    const auto *monitor = selectedMonitor();
    return monitor ? resolutionsFor(*monitor) : QStringList();
}

QStringList DisplaysController::refreshRates() const
{
    const auto *monitor = selectedMonitor();
    return monitor ? refreshRatesFor(*monitor, selectedResolution()) : QStringList();
}

QString DisplaysController::selectedResolution() const
{
    return modeResolution(currentMode());
}

void DisplaysController::setSelectedResolution(const QString &value)
{
    auto *settings = selectedSettings();
    const auto *monitor = selectedMonitor();
    if (!settings || !monitor)
        return;

    const QStringList rates = refreshRatesFor(*monitor, value.trimmed());
    if (rates.isEmpty())
        return;
    const double currentRate = modeRefreshRate(settings->mode);
    const double rate = rates.contains(formatRefreshRate(currentRate)) ? currentRate : rates.first().toDouble();
    const QString mode = modeFor(*monitor, value.trimmed(), rate);
    if (mode.isEmpty() || mode == settings->mode)
        return;
    settings->mode = mode;
    Q_EMIT selectedSettingsChanged();
}

double DisplaysController::selectedRefreshRate() const
{
    return modeRefreshRate(currentMode());
}

void DisplaysController::setSelectedRefreshRate(double value)
{
    auto *settings = selectedSettings();
    const auto *monitor = selectedMonitor();
    if (!settings || !monitor)
        return;
    const QString mode = modeFor(*monitor, selectedResolution(), value);
    if (mode.isEmpty() || mode == settings->mode)
        return;
    settings->mode = mode;
    Q_EMIT selectedSettingsChanged();
}

int DisplaysController::scalePercent() const
{
    const auto *settings = selectedSettings();
    return settings ? qRound(settings->scale * 100.0) : 100;
}

void DisplaysController::setScalePercent(int value)
{
    auto *settings = selectedSettings();
    if (!settings)
        return;
    value = qBound(50, value, 300);
    const double scale = value / 100.0;
    if (qFuzzyCompare(settings->scale, scale))
        return;
    settings->scale = scale;
    Q_EMIT selectedSettingsChanged();
}

int DisplaysController::positionX() const
{
    const auto *settings = selectedSettings();
    return settings ? settings->x : 0;
}

void DisplaysController::setPositionX(int value)
{
    auto *settings = selectedSettings();
    if (!settings || settings->x == value)
        return;
    settings->x = value;
    Q_EMIT selectedSettingsChanged();
}

int DisplaysController::positionY() const
{
    const auto *settings = selectedSettings();
    return settings ? settings->y : 0;
}

void DisplaysController::setPositionY(int value)
{
    auto *settings = selectedSettings();
    if (!settings || settings->y == value)
        return;
    settings->y = value;
    Q_EMIT selectedSettingsChanged();
}

QVariantList DisplaysController::unusedEntries() const
{
    return m_unusedEntries;
}

bool DisplaysController::hasUnusedEntries() const
{
    return !m_unusedEntries.isEmpty();
}

void DisplaysController::setAutomatic(bool value)
{
    if (m_automatic == value)
        return;
    m_automatic = value;
    Q_EMIT settingsChanged();
}

QString DisplaysController::assignmentValue(const QString &line, const QString &key)
{
    const QRegularExpression expression(
        QStringLiteral(R"(^[ \t]*%1[ \t]*=[ \t]*(.*?)\s*$)").arg(QRegularExpression::escape(key)));
    const auto match = expression.match(line);
    return match.hasMatch() ? match.captured(1).trimmed() : QString();
}

QString DisplaysController::unquote(const QString &value)
{
    QString result = value.trimmed();
    if (result.size() >= 2 && result.startsWith(QLatin1Char('"')) && result.endsWith(QLatin1Char('"')))
        result = result.mid(1, result.size() - 2);
    return result;
}

QString DisplaysController::modeResolution(const QString &mode)
{
    const auto match = QRegularExpression(QStringLiteral(R"(^([0-9]+x[0-9]+)@)")).match(mode);
    return match.hasMatch() ? match.captured(1) : QString();
}

double DisplaysController::modeRefreshRate(const QString &mode)
{
    bool ok = false;
    const auto match = QRegularExpression(QStringLiteral(R"(@([0-9]+(?:\.[0-9]+)?)Hz$)")).match(mode);
    const double value = match.hasMatch() ? match.captured(1).toDouble(&ok) : 0.0;
    return ok ? value : 0.0;
}

void DisplaysController::loadConfiguration()
{
    m_configAvailable = QFileInfo::exists(m_configPath) && QFileInfo(m_configPath).isFile();
    m_automatic = true;
    m_hyprConfigPath = defaultHyprConfigPath();
    m_internalOutput = QStringLiteral("eDP-1");

    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString section;
    for (const QString &line : QString::fromUtf8(file.readAll()).split(QLatin1Char('\n')))
    {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith(QLatin1Char('[')) && trimmed.endsWith(QLatin1Char(']')))
        {
            section = trimmed.mid(1, trimmed.size() - 2);
            continue;
        }
        if (section == QStringLiteral("General"))
        {
            const QString hyprConfig = assignmentValue(line, QStringLiteral("HyprConfig"));
            if (!hyprConfig.isEmpty())
                m_hyprConfigPath = expandHome(unquote(hyprConfig));

            const QString automatic = assignmentValue(line, QStringLiteral("Automatic")).toLower();
            if (!automatic.isEmpty())
                m_automatic = automatic == QStringLiteral("1") || automatic == QStringLiteral("true")
                    || automatic == QStringLiteral("yes") || automatic == QStringLiteral("on");
        }
        else if (section == QStringLiteral("Screens"))
        {
            const QString internal = assignmentValue(line, QStringLiteral("InternalScreen"));
            if (!internal.isEmpty())
                m_internalOutput = unquote(internal);
        }
    }
}

void DisplaysController::loadMonitors()
{
    m_monitors.clear();
    m_monitorsVariant.clear();
    m_manualSettings.clear();

    QProcess process;
    process.start(QStringLiteral("hyprctl"), {QStringLiteral("monitors"), QStringLiteral("-j")});
    if (!process.waitForFinished(3000) || process.exitCode() != 0)
        return;

    const QJsonDocument document = QJsonDocument::fromJson(process.readAllStandardOutput());
    if (!document.isArray())
        return;

    for (const auto &entry : document.array())
    {
        const auto object = entry.toObject();
        Monitor monitor;
        monitor.output = object.value(QStringLiteral("name")).toString();
        monitor.description = object.value(QStringLiteral("description")).toString();
        monitor.scale = object.value(QStringLiteral("scale")).toDouble(1.0);
        monitor.x = object.value(QStringLiteral("x")).toInt();
        monitor.y = object.value(QStringLiteral("y")).toInt();
        monitor.width = object.value(QStringLiteral("width")).toInt();
        monitor.height = object.value(QStringLiteral("height")).toInt();
        if (monitor.output.isEmpty())
            continue;

        for (const auto &modeEntry : object.value(QStringLiteral("availableModes")).toArray())
        {
            const QString value = modeEntry.toString();
            const QString resolution = modeResolution(value);
            const double refreshRate = modeRefreshRate(value);
            if (!resolution.isEmpty() && refreshRate > 0.0)
                monitor.modes.append({value, resolution, refreshRate});
        }

        const QString activeResolution = QStringLiteral("%1x%2")
            .arg(object.value(QStringLiteral("width")).toInt())
            .arg(object.value(QStringLiteral("height")).toInt());
        const double activeRefreshRate = object.value(QStringLiteral("refreshRate")).toDouble();
        for (const Mode &mode : monitor.modes)
            if (mode.resolution == activeResolution && qAbs(mode.refreshRate - activeRefreshRate) < 0.1)
                monitor.currentMode = mode.value;
        if (monitor.currentMode.isEmpty() && !monitor.modes.isEmpty())
            monitor.currentMode = monitor.modes.first().value;
        if (monitor.currentMode.isEmpty() && !activeResolution.startsWith(QStringLiteral("0x")))
        {
            monitor.currentMode = activeResolution + QStringLiteral("@") + formatRefreshRate(activeRefreshRate) + QStringLiteral("Hz");
            monitor.modes.append({monitor.currentMode, activeResolution, activeRefreshRate});
        }
        if (monitor.currentMode.isEmpty())
            continue;

        m_manualSettings.insert(monitor.output, {monitor.currentMode, monitor.scale, monitor.x, monitor.y});
        m_monitors.append(monitor);
        QVariantMap monitorValue;
        monitorValue.insert(QStringLiteral("name"), monitor.output);
        monitorValue.insert(QStringLiteral("description"), monitor.description);
        monitorValue.insert(QStringLiteral("x"), monitor.x);
        monitorValue.insert(QStringLiteral("y"), monitor.y);
        monitorValue.insert(QStringLiteral("width"), monitor.width);
        monitorValue.insert(QStringLiteral("height"), monitor.height);
        monitorValue.insert(QStringLiteral("scale"), monitor.scale);
        const QString monitorType = monitor.output == m_internalOutput ? QStringLiteral("Internal") : QStringLiteral("External");
        monitorValue.insert(QStringLiteral("type"), monitorType);
        monitorValue.insert(QStringLiteral("display"), QStringLiteral("%1 · %2").arg(monitor.output, monitorType));
        m_monitorsVariant.append(monitorValue);
    }

    if (m_selectedMonitorIndex >= m_monitors.size())
        m_selectedMonitorIndex = qMax(0, m_monitors.size() - 1);
}

void DisplaysController::loadConfiguredEntries()
{
    m_removedOutputs.clear();
    m_unusedEntries.clear();
    QFile source(m_hyprConfigPath);
    if (!source.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QSet<QString> connectedOutputs;
    for (const Monitor &monitor : m_monitors)
        connectedOutputs.insert(monitor.output);

    const QString text = QString::fromUtf8(source.readAll());
    const QRegularExpression blockExpression(QStringLiteral(R"(hl\.monitor\s*\(\s*\{[\s\S]*?\}\s*\))"));
    QSet<QString> seen;
    auto matches = blockExpression.globalMatch(text);
    while (matches.hasNext())
    {
        const QString block = matches.next().captured(0);
        const QString output = blockValue(block, QStringLiteral("output"));
        if (output.isEmpty() || connectedOutputs.contains(output) || seen.contains(output))
            continue;
        seen.insert(output);
        QVariantMap entry;
        entry.insert(QStringLiteral("output"), output);
        entry.insert(QStringLiteral("mode"), blockValue(block, QStringLiteral("mode")));
        entry.insert(QStringLiteral("position"), blockValue(block, QStringLiteral("position")));
        m_unusedEntries.append(entry);
    }
}

void DisplaysController::reload()
{
    loadConfiguration();
    loadMonitors();
    loadConfiguredEntries();
    Q_EMIT settingsChanged();
    Q_EMIT monitorsChanged();
    Q_EMIT selectedMonitorChanged();
    Q_EMIT selectedSettingsChanged();
}

QStringList DisplaysController::resolutionsFor(const Monitor &monitor) const
{
    QStringList result;
    for (const Mode &mode : monitor.modes)
        if (!result.contains(mode.resolution))
            result.append(mode.resolution);
    return result;
}

QStringList DisplaysController::refreshRatesFor(const Monitor &monitor, const QString &resolution) const
{
    QStringList result;
    for (const Mode &mode : monitor.modes)
        if (mode.resolution == resolution && !result.contains(formatRefreshRate(mode.refreshRate)))
            result.append(formatRefreshRate(mode.refreshRate));
    return result;
}

QString DisplaysController::modeFor(const Monitor &monitor, const QString &resolution, double refreshRate) const
{
    for (const Mode &mode : monitor.modes)
        if (mode.resolution == resolution && qAbs(mode.refreshRate - refreshRate) < 0.1)
            return mode.value;
    return QString();
}

QString DisplaysController::currentMode() const
{
    const auto *settings = selectedSettings();
    return settings ? settings->mode : QString();
}

DisplaysController::ManualSettings *DisplaysController::selectedSettings()
{
    const auto *monitor = selectedMonitor();
    return monitor ? &m_manualSettings[monitor->output] : nullptr;
}

const DisplaysController::ManualSettings *DisplaysController::selectedSettings() const
{
    const auto *monitor = selectedMonitor();
    if (!monitor)
        return nullptr;
    const auto iterator = m_manualSettings.constFind(monitor->output);
    return iterator == m_manualSettings.cend() ? nullptr : &iterator.value();
}

const DisplaysController::Monitor *DisplaysController::selectedMonitor() const
{
    return m_selectedMonitorIndex >= 0 && m_selectedMonitorIndex < m_monitors.size()
        ? &m_monitors.at(m_selectedMonitorIndex) : nullptr;
}

bool DisplaysController::writeHyprscreendConfiguration() const
{
    const QFileInfo fileInfo(m_configPath);
    if (!QDir().mkpath(fileInfo.absolutePath()))
        return false;

    QString text;
    QFile source(m_configPath);
    if (source.open(QIODevice::ReadOnly | QIODevice::Text))
        text = QString::fromUtf8(source.readAll());
    if (text.isEmpty())
        text = QStringLiteral("[General]\nHyprConfig=\"%1\"\nAutomatic=true\nScaleFactor=1.0\n\n[Screens]\nInternalScreen=\"eDP-1\"\nExternalScreen=\"\"\n")
            .arg(m_hyprConfigPath);

    QStringList lines = text.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
    bool inGeneral = false;
    bool updated = false;
    for (QString &line : lines)
    {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith(QLatin1Char('[')) && trimmed.endsWith(QLatin1Char(']')))
            inGeneral = trimmed.mid(1, trimmed.size() - 2) == QStringLiteral("General");
        else if (inGeneral && !assignmentValue(line, QStringLiteral("Automatic")).isEmpty())
        {
            const int indentation = line.indexOf(QRegularExpression(QStringLiteral("\\S")));
            line = QString(qMax(0, indentation), QLatin1Char(' '))
                + QStringLiteral("Automatic=") + (m_automatic ? QStringLiteral("true") : QStringLiteral("false"));
            updated = true;
        }
    }
    if (!updated)
    {
        int generalIndex = -1;
        for (int i = 0; i < lines.size(); ++i)
            if (lines.at(i).trimmed() == QStringLiteral("[General]")) { generalIndex = i; break; }
        if (generalIndex >= 0)
            lines.insert(generalIndex + 1, QStringLiteral("Automatic=") + (m_automatic ? QStringLiteral("true") : QStringLiteral("false")));
        else
            lines.append({QString(), QStringLiteral("[General]"), QStringLiteral("Automatic=") + (m_automatic ? QStringLiteral("true") : QStringLiteral("false"))});
    }

    QSaveFile destination(m_configPath);
    if (!destination.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    const QByteArray data = lines.join(QLatin1Char('\n')).toUtf8();
    return destination.write(data) == data.size() && destination.commit();
}

bool DisplaysController::updateMonitorBlock(const QString &output, const ManualSettings &settings) const
{
    QFile source(m_hyprConfigPath);
    if (!source.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    const QString text = QString::fromUtf8(source.readAll());
    const QRegularExpression blockExpression(QStringLiteral(R"(hl\.monitor\s*\(\s*\{[\s\S]*?\}\s*\))"));
    auto matches = blockExpression.globalMatch(text);
    QRegularExpressionMatch selected;
    QString position = QStringLiteral("%1 %2").arg(settings.x).arg(settings.y);
    while (matches.hasNext())
    {
        const auto match = matches.next();
        const QString block = match.captured(0);
        const QString blockOutput = blockValue(block, QStringLiteral("output"));
        if (blockOutput == output || (!selected.hasMatch() && blockOutput.isEmpty()))
        {
            selected = match;
            if (blockOutput == output)
                break;
        }
    }

    const qsizetype lineStart = selected.hasMatch() ? text.lastIndexOf(QLatin1Char('\n'), selected.capturedStart() - 1) : -1;
    const QString indent = selected.hasMatch() ? text.mid(lineStart + 1, selected.capturedStart() - lineStart - 1) : QString();
    const QString replacement = indent + QStringLiteral("hl.monitor({\n")
        + indent + QStringLiteral("    output = \"") + luaEscape(output) + QStringLiteral("\",\n")
        + indent + QStringLiteral("    mode = \"") + luaEscape(settings.mode) + QStringLiteral("\",\n")
        + indent + QStringLiteral("    position = \"") + luaEscape(position.isEmpty() ? QStringLiteral("auto") : position) + QStringLiteral("\",\n")
        + indent + QStringLiteral("    scale = ") + QString::number(settings.scale, 'f', 2) + QStringLiteral(",\n")
        + indent + QStringLiteral("})");

    QString result;
    if (selected.hasMatch())
        result = text.left(selected.capturedStart()) + replacement + text.mid(selected.capturedEnd());
    else
        result = replacement + (text.isEmpty() ? QStringLiteral("\n") : QStringLiteral("\n") + text);

    QSaveFile destination(m_hyprConfigPath);
    if (!destination.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    const QByteArray data = result.toUtf8();
    return destination.write(data) == data.size() && destination.commit();
}

bool DisplaysController::removeMonitorBlock(const QString &output) const
{
    QFile source(m_hyprConfigPath);
    if (!source.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    const QString text = QString::fromUtf8(source.readAll());
    const QRegularExpression blockExpression(QStringLiteral(R"(hl\.monitor\s*\(\s*\{[\s\S]*?\}\s*\))"));
    auto matches = blockExpression.globalMatch(text);
    while (matches.hasNext())
    {
        const auto match = matches.next();
        if (blockValue(match.captured(0), QStringLiteral("output")) != output)
            continue;

        qsizetype start = match.capturedStart();
        qsizetype end = match.capturedEnd();
        const qsizetype lineStart = text.lastIndexOf(QLatin1Char(10), start - 1) + 1;
        if (text.mid(lineStart, start - lineStart).trimmed().isEmpty())
            start = lineStart;
        if (end < text.size() && text.at(end) == QLatin1Char(10))
            ++end;

        QSaveFile destination(m_hyprConfigPath);
        if (!destination.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        const QByteArray data = text.left(start).toUtf8() + text.mid(end).toUtf8();
        return destination.write(data) == data.size() && destination.commit();
    }
    return false;
}

void DisplaysController::removeUnusedEntry(const QString &output)
{
    for (int i = 0; i < m_unusedEntries.size(); ++i)
    {
        if (m_unusedEntries.at(i).toMap().value(QStringLiteral("output")).toString() != output)
            continue;
        m_unusedEntries.removeAt(i);
        m_removedOutputs.insert(output);
        Q_EMIT settingsChanged();
        return;
    }
}

bool DisplaysController::save()
{
    if (!m_available || !writeHyprscreendConfiguration())
        return false;

    bool monitorConfigChanged = false;
    for (const QString &output : m_removedOutputs)
    {
        if (!removeMonitorBlock(output))
            return false;
        monitorConfigChanged = true;
    }

    if (!m_automatic)
    {
        for (const Monitor &monitor : m_monitors)
        {
            const ManualSettings settings = m_manualSettings.value(monitor.output);
            if (settings.mode.isEmpty() || !updateMonitorBlock(monitor.output, settings))
                return false;
        }
        monitorConfigChanged = true;
    }

    if (monitorConfigChanged)
        QProcess::execute(QStringLiteral("hyprctl"), {QStringLiteral("reload")});
    m_removedOutputs.clear();
    loadConfiguredEntries();
    Q_EMIT settingsChanged();
    return true;
}
