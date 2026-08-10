#include "desklockcontroller.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QSaveFile>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
#include <QtGlobal>

namespace
{
constexpr auto defaultWallpaper = "/usr/share/wallpapers/Aqua/contents/images/2560x1440.png";

QString desklockConfigPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + QStringLiteral("/desklock/desklock.conf");
}

QString hypridleConfigPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + QStringLiteral("/hypr/hypridle.conf");
}

constexpr auto defaultHypridleLockTimeout = 350;

void restartHypridle(const QString &configPath)
{
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("hypridle"));
    if (executable.isEmpty())
    {
        qWarning() << "Could not find hypridle to restart it";
        return;
    }

    QProcess::execute(QStringLiteral("pkill"), {QStringLiteral("-TERM"), QStringLiteral("-x"), QStringLiteral("hypridle")});
    if (!QProcess::startDetached(executable, {QStringLiteral("--config"), configPath}))
        qWarning() << "Could not restart hypridle";
}

bool isHypridleCommand(const QString &line, const QString &command)
{
    if (command == QStringLiteral("dim"))
        return line.contains(QStringLiteral("on-timeout"))
            && line.contains(QStringLiteral("brightnessctl -s set 10000"));
    if (command == QStringLiteral("lock"))
        return line.contains(QStringLiteral("on-timeout"))
            && (line.contains(QStringLiteral("on-timeout = desklock"))
                || line.contains(QStringLiteral("pidof desklock || desklock")));
    if (command == QStringLiteral("dpms"))
        return line.contains(QStringLiteral("on-timeout"))
            && (line.contains(QStringLiteral("hyprctl dispatch dpms off"))
                || line.contains(QStringLiteral("hl.dsp.dpms({action = \"off\"})")));
    if (command == QStringLiteral("suspend"))
        return line.contains(QStringLiteral("on-timeout"))
            && (line.contains(QStringLiteral("on-timeout = zzz"))
                || line.contains(QStringLiteral("on-timeout = loginctl suspend")));
    return false;
}

QString uncommentedHypridleLine(const QString &line)
{
    QString result = line;
    int indentation = 0;
    while (indentation < result.size() && result.at(indentation).isSpace())
        ++indentation;
    if (result.mid(indentation).startsWith(QStringLiteral("#")))
    {
        result.remove(indentation, 1);
        if (indentation < result.size() && result.at(indentation) == QLatin1Char(' '))
            result.remove(indentation, 1);
    }
    return result;
}

struct HypridleListener
{
    int start = -1;
    int end = -1;
    int timeoutLine = -1;
    int commandLine = -1;
};

HypridleListener findHypridleListener(const QStringList &lines, const QString &command)
{
    for (int start = 0; start < lines.size(); ++start)
    {
        if (uncommentedHypridleLine(lines[start]).trimmed() != QStringLiteral("listener {"))
            continue;

        int depth = 0;
        int end = -1;
        for (int i = start; i < lines.size(); ++i)
        {
            const QString normalized = uncommentedHypridleLine(lines[i]).trimmed();
            if (normalized.endsWith(QLatin1Char('{')))
                ++depth;
            else if (normalized == QStringLiteral("}"))
            {
                --depth;
                if (depth == 0)
                {
                    end = i;
                    break;
                }
            }
        }
        if (end < 0)
            continue;

        HypridleListener listener{start, end};
        for (int i = start + 1; i < end; ++i)
        {
            const QString normalized = uncommentedHypridleLine(lines[i]).trimmed();
            const int equals = normalized.indexOf(QLatin1Char('='));
            if (equals >= 0 && normalized.left(equals).trimmed() == QStringLiteral("timeout"))
                listener.timeoutLine = i;
            if (isHypridleCommand(normalized, command))
                listener.commandLine = i;
        }

        if (listener.commandLine >= 0)
            return listener;
    }

    return {};
}

bool updateIniValue(QStringList &lines, const QString &section, const QString &key, const QString &value)
{
    QString currentSection;
    int sectionEnd = lines.size();
    int sectionStart = -1;
    for (int i = 0; i < lines.size(); ++i)
    {
        const QString trimmed = lines[i].trimmed();
        if (trimmed.startsWith(QLatin1Char('[')) && trimmed.endsWith(QLatin1Char(']')))
        {
            if (currentSection == section)
            {
                sectionEnd = i;
                break;
            }
            currentSection = trimmed.mid(1, trimmed.size() - 2);
            if (currentSection == section)
                sectionStart = i;
        }
        if (currentSection != section || trimmed.startsWith(QLatin1Char('#')) || !trimmed.contains(QLatin1Char('=')))
            continue;
        const int equals = lines[i].indexOf(QLatin1Char('='));
        if (lines[i].left(equals).trimmed() != key)
            continue;
        int valueStart = equals + 1;
        while (valueStart < lines[i].size() && lines[i].at(valueStart).isSpace())
            ++valueStart;
        lines[i] = lines[i].left(equals + 1) + lines[i].mid(equals + 1, valueStart - equals - 1) + value;
        return true;
    }
    if (sectionStart < 0)
    {
        if (!lines.isEmpty() && !lines.last().isEmpty())
            lines.append(QString());
        lines.append(QStringLiteral("[%1]").arg(section));
        lines.append(QStringLiteral("%1=%2").arg(key, value));
    }
    else
    {
        lines.insert(sectionEnd, QStringLiteral("%1=%2").arg(key, value));
    }
    return true;
}
}

DesklockController::DesklockController(QObject *parent)
    : QObject(parent)
    , m_configPath(desklockConfigPath())
    , m_hypridleConfigPath(hypridleConfigPath())
{
    load();
}

QString DesklockController::configPath() const { return m_configPath; }

QString DesklockController::wallpaperDirectory() const
{
    if (m_wallpaperPath.isEmpty())
        return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    return QFileInfo(m_wallpaperPath).absolutePath();
}

QString DesklockController::wallpaperPath() const { return m_wallpaperPath; }

QString DesklockController::avatarDirectory() const
{
    if (m_avatarPath.isEmpty() || m_avatarPath.contains(QStringLiteral("%u"))
        || m_avatarPath.contains(QStringLiteral("%h")))
        return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    return QFileInfo(m_avatarPath).absolutePath();
}

QString DesklockController::avatarPath() const { return m_avatarPath; }
QString DesklockController::iconMode() const { return m_iconMode; }
bool DesklockController::blurEnabled() const { return m_blurEnabled; }
bool DesklockController::overlayEnabled() const { return m_overlayEnabled; }
double DesklockController::overlayOpacity() const { return m_overlayOpacity; }
QString DesklockController::timeFormat() const { return m_timeFormat; }
QString DesklockController::dateFormat() const { return m_dateFormat; }
bool DesklockController::lowercaseDate() const { return m_lowercaseDate; }
bool DesklockController::showSystemMonitor() const { return m_showSystemMonitor; }
bool DesklockController::showBattery() const { return m_showBattery; }
int DesklockController::batteryUpdateInterval() const { return m_batteryUpdateInterval; }
bool DesklockController::showMediaPlayer() const { return m_showMediaPlayer; }
int DesklockController::systemMonitorUpdateInterval() const { return m_systemMonitorUpdateInterval; }
bool DesklockController::hideCursor() const { return m_hideCursor; }
int DesklockController::dimTimeout() const { return m_dimTimeout; }
bool DesklockController::idleLockEnabled() const { return m_idleLockEnabled; }
int DesklockController::idleLockTimeout() const { return m_idleLockTimeout; }
int DesklockController::dpmsTimeout() const { return m_dpmsTimeout; }
int DesklockController::suspendTimeout() const { return m_suspendTimeout; }

QString DesklockController::normalizeLocalPath(const QString &value)
{
    QString path = value.trimmed();
    if (path.startsWith(QStringLiteral("file:")))
        path = QUrl(path).toLocalFile();

    return path.isEmpty() ? QString() : QFileInfo(path).absoluteFilePath();
}

void DesklockController::notifyDesklock(const QString &key)
{
    QDBusMessage message = QDBusMessage::createSignal(
        QStringLiteral("/org/nitrux/desklock"),
        QStringLiteral("org.nitrux.desklock.Settings"),
        QStringLiteral("ConfigurationChanged"));
    message << m_configPath << key;
    QDBusConnection::sessionBus().send(message);
}

void DesklockController::setWallpaperPath(const QString &value)
{
    const QString normalized = normalizeLocalPath(value);
    if (m_wallpaperPath == normalized)
        return;

    m_wallpaperPath = normalized;
    Q_EMIT wallpaperPathChanged();
}

void DesklockController::setAvatarPath(const QString &value)
{
    const QString trimmed = value.trimmed();
    const QString normalized = trimmed.contains(QStringLiteral("%u"))
        || trimmed.contains(QStringLiteral("%h"))
        ? trimmed : normalizeLocalPath(trimmed);
    if (m_avatarPath == normalized)
        return;

    m_avatarPath = normalized;
    Q_EMIT avatarPathChanged();
}

void DesklockController::setIconMode(const QString &value)
{
    const QString normalized = value.trimmed().toLower() == QStringLiteral("nerd")
        ? QStringLiteral("nerd") : QStringLiteral("system");
    if (m_iconMode == normalized)
        return;
    m_iconMode = normalized;
    Q_EMIT iconModeChanged();
}

void DesklockController::setOverlayOpacity(double value)
{
    value = qBound(0.0, value, 1.0);
    if (qFuzzyCompare(m_overlayOpacity, value))
        return;

    m_overlayOpacity = value;
    Q_EMIT overlayOpacityChanged();
}

void DesklockController::setTimeFormat(const QString &value)
{
    const QString normalized = value.trimmed().isEmpty() ? QStringLiteral("hh:mm") : value.trimmed();
    if (m_timeFormat == normalized)
        return;

    m_timeFormat = normalized;
    Q_EMIT timeFormatChanged();
}

void DesklockController::setDateFormat(const QString &value)
{
    const QString normalized = value.trimmed().isEmpty()
        ? QStringLiteral("dddd, dd MMMM yyyy")
        : value.trimmed();
    if (m_dateFormat == normalized)
        return;

    m_dateFormat = normalized;
    Q_EMIT dateFormatChanged();
}

#define SET_BOUNDED_SETTING(Method, Member, Signal, Minimum, Maximum) \
    void DesklockController::Method(int value) \
    { \
        value = qBound(Minimum, value, Maximum); \
        if (Member == value) \
            return; \
        Member = value; \
        Q_EMIT Signal(); \
    }

SET_BOUNDED_SETTING(setDimTimeout, m_dimTimeout, dimTimeoutChanged, 0, 86400)
void DesklockController::setIdleLockTimeout(int value)
{
    value = qBound(0, value, 86400);
    if (m_idleLockTimeout == value)
        return;
    m_idleLockTimeout = value;
    if (value == 0 && m_idleLockEnabled) {
        m_idleLockEnabled = false;
        Q_EMIT idleLockEnabledChanged();
    }
    Q_EMIT idleLockTimeoutChanged();
}

SET_BOUNDED_SETTING(setDpmsTimeout, m_dpmsTimeout, dpmsTimeoutChanged, 0, 86400)
SET_BOUNDED_SETTING(setSuspendTimeout, m_suspendTimeout, suspendTimeoutChanged, 0, 86400)
SET_BOUNDED_SETTING(setBatteryUpdateInterval, m_batteryUpdateInterval, batteryUpdateIntervalChanged, 1000, 3600000)
SET_BOUNDED_SETTING(setSystemMonitorUpdateInterval, m_systemMonitorUpdateInterval, systemMonitorUpdateIntervalChanged, 1000, 3600000)

#undef SET_BOUNDED_SETTING

#define SET_BOOLEAN_SETTING(Method, Member, Signal) \
    void DesklockController::Method(bool value) \
    { \
        if (Member == value) \
            return; \
        Member = value; \
        Q_EMIT Signal(); \
    }

SET_BOOLEAN_SETTING(setShowSystemMonitor, m_showSystemMonitor, showSystemMonitorChanged)
SET_BOOLEAN_SETTING(setShowBattery, m_showBattery, showBatteryChanged)
SET_BOOLEAN_SETTING(setShowMediaPlayer, m_showMediaPlayer, showMediaPlayerChanged)
SET_BOOLEAN_SETTING(setLowercaseDate, m_lowercaseDate, lowercaseDateChanged)
SET_BOOLEAN_SETTING(setHideCursor, m_hideCursor, hideCursorChanged)
SET_BOOLEAN_SETTING(setIdleLockEnabled, m_idleLockEnabled, idleLockEnabledChanged)
SET_BOOLEAN_SETTING(setBlurEnabled, m_blurEnabled, blurEnabledChanged)
SET_BOOLEAN_SETTING(setOverlayEnabled, m_overlayEnabled, overlayEnabledChanged)

#undef SET_BOOLEAN_SETTING

void DesklockController::reload()
{
    load();
}

bool DesklockController::save()
{
    const QFileInfo configInfo(m_configPath);
    if (!QDir().mkpath(configInfo.absolutePath()))
    {
        qWarning() << "Could not create the Desklock configuration directory" << configInfo.absolutePath();
        return false;
    }

    const auto writeSettings = [this]() {
        QFile source(m_configPath);
        QStringList lines;
        if (source.exists())
        {
            if (!source.open(QIODevice::ReadOnly | QIODevice::Text))
                return QSettings::AccessError;
            lines = QString::fromUtf8(source.readAll()).split(QStringLiteral("\n"));
        }

        updateIniValue(lines, QStringLiteral("Appearance"), QStringLiteral("BackgroundImage"), m_wallpaperPath);
        updateIniValue(lines, QStringLiteral("Appearance"), QStringLiteral("AvatarImage"), m_avatarPath);
        updateIniValue(lines, QStringLiteral("Appearance"), QStringLiteral("IconMode"), m_iconMode);
        updateIniValue(lines, QStringLiteral("Appearance"), QStringLiteral("BlurEnabled"), m_blurEnabled ? QStringLiteral("true") : QStringLiteral("false"));
        updateIniValue(lines, QStringLiteral("Appearance"), QStringLiteral("OverlayEnabled"), m_overlayEnabled ? QStringLiteral("true") : QStringLiteral("false"));
        updateIniValue(lines, QStringLiteral("Appearance"), QStringLiteral("OverlayOpacity"), QString::number(m_overlayOpacity, 'g', 17));
        updateIniValue(lines, QStringLiteral("Clock"), QStringLiteral("TimeFormat"), m_timeFormat);
        updateIniValue(lines, QStringLiteral("Clock"), QStringLiteral("DateFormat"), m_dateFormat);
        updateIniValue(lines, QStringLiteral("Clock"), QStringLiteral("LowercaseDate"), m_lowercaseDate ? QStringLiteral("true") : QStringLiteral("false"));
        updateIniValue(lines, QStringLiteral("SystemMonitor"), QStringLiteral("Enabled"), m_showSystemMonitor ? QStringLiteral("true") : QStringLiteral("false"));
        updateIniValue(lines, QStringLiteral("Battery"), QStringLiteral("Enabled"), m_showBattery ? QStringLiteral("true") : QStringLiteral("false"));
        updateIniValue(lines, QStringLiteral("Battery"), QStringLiteral("UpdateInterval"), QString::number(m_batteryUpdateInterval));
        updateIniValue(lines, QStringLiteral("Media"), QStringLiteral("Enabled"), m_showMediaPlayer ? QStringLiteral("true") : QStringLiteral("false"));
        updateIniValue(lines, QStringLiteral("SystemMonitor"), QStringLiteral("UpdateInterval"), QString::number(m_systemMonitorUpdateInterval));
        updateIniValue(lines, QStringLiteral("Behavior"), QStringLiteral("HideCursor"), m_hideCursor ? QStringLiteral("true") : QStringLiteral("false"));

        QSaveFile destination(m_configPath);
        if (!destination.open(QIODevice::WriteOnly | QIODevice::Text)
            || destination.write(lines.join(QStringLiteral("\n")).toUtf8()) < 0
            || !destination.commit())
            return QSettings::AccessError;
        return QSettings::NoError;
    };

    QSettings::Status status = writeSettings();

    if (status != QSettings::NoError)
    {
        qWarning() << "Could not write Desklock configuration" << m_configPath
                   << "QSettings status" << static_cast<int>(status);
        return false;
    }

    if (!saveHypridleConfiguration())
        return false;

    notifyDesklock(QStringLiteral("*"));
    Q_EMIT configurationChanged(QStringLiteral("*"));
    return true;
}

void DesklockController::load()
{
    QSettings settings(m_configPath, QSettings::IniFormat);

    m_wallpaperPath = normalizeLocalPath(settings.value(
        QStringLiteral("Appearance/BackgroundImage"),
        QString::fromLatin1(defaultWallpaper)).toString());
    m_avatarPath = settings.value(QStringLiteral("Appearance/AvatarImage")).toString().trimmed();
    m_iconMode = settings.value(QStringLiteral("Appearance/IconMode"), QStringLiteral("system")).toString().trimmed().toLower() == QStringLiteral("nerd")
        ? QStringLiteral("nerd") : QStringLiteral("system");
    m_blurEnabled = settings.value(QStringLiteral("Appearance/BlurEnabled"), true).toBool();
    m_overlayEnabled = settings.value(QStringLiteral("Appearance/OverlayEnabled"), true).toBool();
    m_overlayOpacity = qBound(0.0, settings.value(
        QStringLiteral("Appearance/OverlayOpacity"), 0.76).toDouble(), 1.0);
    m_timeFormat = settings.value(QStringLiteral("Clock/TimeFormat"), QStringLiteral("hh:mm")).toString();
    m_dateFormat = settings.value(
        QStringLiteral("Clock/DateFormat"),
        QStringLiteral("dddd, dd MMMM yyyy")).toString();
    m_lowercaseDate = settings.value(QStringLiteral("Clock/LowercaseDate"), false).toBool();
    m_showSystemMonitor = settings.value(QStringLiteral("SystemMonitor/Enabled"), true).toBool();
    m_showBattery = settings.value(QStringLiteral("Battery/Enabled"), true).toBool();
    m_batteryUpdateInterval = qBound(1000, settings.value(
        QStringLiteral("Battery/UpdateInterval"), 30000).toInt(), 3600000);
    m_showMediaPlayer = settings.value(QStringLiteral("Media/Enabled"), true).toBool();
    m_systemMonitorUpdateInterval = qBound(1000, settings.value(
        QStringLiteral("SystemMonitor/UpdateInterval"), 3000).toInt(), 3600000);
    m_hideCursor = settings.value(QStringLiteral("Behavior/HideCursor"), true).toBool();
    loadHypridleConfiguration();

    Q_EMIT wallpaperPathChanged();
    Q_EMIT avatarPathChanged();
    Q_EMIT iconModeChanged();
    Q_EMIT blurEnabledChanged();
    Q_EMIT overlayEnabledChanged();
    Q_EMIT overlayOpacityChanged();
    Q_EMIT timeFormatChanged();
    Q_EMIT dateFormatChanged();
    Q_EMIT lowercaseDateChanged();
    Q_EMIT showSystemMonitorChanged();
    Q_EMIT showBatteryChanged();
    Q_EMIT batteryUpdateIntervalChanged();
    Q_EMIT showMediaPlayerChanged();
    Q_EMIT systemMonitorUpdateIntervalChanged();
    Q_EMIT hideCursorChanged();
}

void DesklockController::loadHypridleConfiguration()
{
    QFile file(m_hypridleConfigPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_dimTimeout = 300;
        m_idleLockEnabled = false;
        m_idleLockTimeout = defaultHypridleLockTimeout;
        m_dpmsTimeout = 500;
        m_suspendTimeout = 600;
    }
    else
    {
        const QStringList lines = QString::fromUtf8(file.readAll()).split(QStringLiteral("\n"));
        const auto uncommentedLine = [](const QString &line) {
            QString result = line;
            int indentation = 0;
            while (indentation < result.size() && result.at(indentation).isSpace())
                ++indentation;
            if (result.mid(indentation).startsWith(QStringLiteral("#")))
            {
                result.remove(indentation, 1);
                if (indentation < result.size() && result.at(indentation) == QLatin1Char(' '))
                    result.remove(indentation, 1);
            }
            return result;
        };
        const auto readTimeout = [&](const QString &command, int fallback) {
            const HypridleListener listener = findHypridleListener(lines, command);
            if (listener.timeoutLine < 0)
                return fallback;
            return uncommentedLine(lines[listener.timeoutLine]).trimmed().section(QStringLiteral("="), 1).trimmed().toInt();
        };

        m_dimTimeout = qBound(0, readTimeout(QStringLiteral("dim"), 300), 86400);
        m_idleLockTimeout = qBound(0, readTimeout(QStringLiteral("lock"), 350), 86400);
        m_dpmsTimeout = qBound(0, readTimeout(QStringLiteral("dpms"), 500), 86400);
        m_suspendTimeout = qBound(0, readTimeout(QStringLiteral("suspend"), 600), 86400);
        m_idleLockEnabled = false;
        for (const QString &line : lines)
        {
            const QString trimmed = uncommentedLine(line).trimmed();
            if (!line.trimmed().startsWith(QStringLiteral("#"))
                && trimmed.startsWith(QStringLiteral("lock_cmd ="))
                && trimmed.contains(QStringLiteral("desklock")))
            {
                m_idleLockEnabled = true;
                break;
            }
        }
    }

    Q_EMIT dimTimeoutChanged();
    Q_EMIT idleLockEnabledChanged();
    Q_EMIT idleLockTimeoutChanged();
    Q_EMIT dpmsTimeoutChanged();
    Q_EMIT suspendTimeoutChanged();
}

bool DesklockController::saveHypridleConfiguration() const
{
    QFile file(m_hypridleConfigPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Could not read hypridle configuration" << m_hypridleConfigPath;
        return false;
    }

    QStringList lines = QString::fromUtf8(file.readAll()).split(QStringLiteral("\n"));
    const auto uncommentedLine = [](const QString &line) {
        QString result = line;
        int indentation = 0;
        while (indentation < result.size() && result.at(indentation).isSpace())
            ++indentation;
        if (result.mid(indentation).startsWith(QStringLiteral("#")))
        {
            result.remove(indentation, 1);
            if (indentation < result.size() && result.at(indentation) == QLatin1Char(' '))
                result.remove(indentation, 1);
        }
        return result;
    };

    const auto updateListener = [&](const QString &command, int timeout) {
        const HypridleListener listener = findHypridleListener(lines, command);
        if (listener.timeoutLine < 0)
            return false;

        lines[listener.timeoutLine] = QStringLiteral("  timeout = %1").arg(timeout);
        return true;
    };

    const auto setBlockEnabled = [&](const QString &command, const QString &blockName, bool enabled) {
        if (blockName == QStringLiteral("listener {"))
        {
            const HypridleListener listener = findHypridleListener(lines, command);
            if (listener.start < 0)
                return false;

            for (int i = listener.start; i <= listener.end; ++i)
            {
                if (enabled)
                    lines[i] = uncommentedLine(lines[i]);
                else if (!lines[i].trimmed().isEmpty())
                    lines[i] = QStringLiteral("# ") + uncommentedLine(lines[i]);
            }
            return true;
        }

        int commandLine = -1;
        for (int i = 0; i < lines.size(); ++i)
        {
            const QString normalized = uncommentedLine(lines[i]).trimmed();
            if (!(normalized.startsWith(QStringLiteral("lock_cmd ="))
                  && normalized.contains(QStringLiteral("desklock"))))
                continue;
            commandLine = i;
            break;
        }
        if (commandLine < 0)
            return false;

        int start = -1;
        for (int i = commandLine; i >= 0; --i)
        {
            if (uncommentedLine(lines[i]).trimmed() == blockName)
            {
                start = i;
                break;
            }
        }
        if (start < 0)
            return false;

        int depth = 0;
        int end = -1;
        for (int i = start; i < lines.size(); ++i)
        {
            const QString normalized = uncommentedLine(lines[i]).trimmed();
            if (normalized.endsWith(QLatin1Char('{')))
                ++depth;
            else if (normalized == QStringLiteral("}"))
            {
                --depth;
                if (depth == 0)
                {
                    end = i;
                    break;
                }
            }
        }
        if (end < 0)
            return false;

        for (int i = start; i <= end; ++i)
        {
            if (enabled)
                lines[i] = uncommentedLine(lines[i]);
            else if (!lines[i].trimmed().isEmpty())
                lines[i] = QStringLiteral("# ") + uncommentedLine(lines[i]);
        }
        return true;
    };

    if (!updateListener(QStringLiteral("dim"), m_dimTimeout)
        || !updateListener(QStringLiteral("lock"), m_idleLockTimeout)
        || !updateListener(QStringLiteral("dpms"), m_dpmsTimeout)
        || !updateListener(QStringLiteral("suspend"), m_suspendTimeout)
        || !setBlockEnabled(QStringLiteral("lock"), QStringLiteral("general {"), m_idleLockEnabled)
        || !setBlockEnabled(QStringLiteral("dim"), QStringLiteral("listener {"), m_idleLockEnabled)
        || !setBlockEnabled(QStringLiteral("lock"), QStringLiteral("listener {"), m_idleLockEnabled)
        || !setBlockEnabled(QStringLiteral("dpms"), QStringLiteral("listener {"), m_idleLockEnabled)
        || !setBlockEnabled(QStringLiteral("suspend"), QStringLiteral("listener {"), m_idleLockEnabled))
    {
        qWarning() << "Could not find all default hypridle blocks in" << m_hypridleConfigPath;
        return false;
    }

    QSaveFile destination(m_hypridleConfigPath);
    if (!destination.open(QIODevice::WriteOnly | QIODevice::Text)
        || destination.write(lines.join(QStringLiteral("\n")).toUtf8()) < 0
        || !destination.commit())
    {
        qWarning() << "Could not write hypridle configuration" << m_hypridleConfigPath;
        return false;
    }
    restartHypridle(m_hypridleConfigPath);
    return true;
}
