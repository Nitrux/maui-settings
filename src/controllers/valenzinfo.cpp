#include "valenzinfo.h"

#include <QDir>
#include <QSettings>
#include <QStringList>
#include <QVariant>
#include <QtGlobal>

namespace
{
constexpr auto distroConfigPath = "/etc/valenz/valenz.conf";

QVariant settingValue(const QSettings &userSettings, const QSettings &distroSettings, const QString &key, const QVariant &fallback)
{
    if (userSettings.contains(key))
        return userSettings.value(key);
    if (distroSettings.contains(key))
        return distroSettings.value(key);
    return fallback;
}

QString normalizedChoice(const QString &value, const QStringList &choices, const QString &fallback)
{
    const QString normalized = value.trimmed().toLower();
    return choices.contains(normalized) ? normalized : fallback;
}
} // namespace

ValenzInfo::ValenzInfo(QObject *parent)
    : QObject(parent)
    , m_configPath(QDir::homePath() + QStringLiteral("/.config/valenz/valenz.conf"))
{
    load();
}

QString ValenzInfo::configPath() const { return m_configPath; }
int ValenzInfo::barHeight() const { return m_barHeight; }
int ValenzInfo::barLayerSpacing() const { return m_barLayerSpacing; }
int ValenzInfo::barLayerSpacingTop() const { return m_barLayerSpacingTop; }
int ValenzInfo::barLayerSpacingBottom() const { return m_barLayerSpacingBottom; }
int ValenzInfo::barLayerSpacingLeft() const { return m_barLayerSpacingLeft; }
int ValenzInfo::barLayerSpacingRight() const { return m_barLayerSpacingRight; }
QString ValenzInfo::screenPlacement() const { return m_screenPlacement; }
QString ValenzInfo::controlCenterIconMode() const { return m_controlCenterIconMode; }
QString ValenzInfo::controlCenterPowerCommand() const { return m_controlCenterPowerCommand; }
QString ValenzInfo::controlCenterSettingsCommand() const { return m_controlCenterSettingsCommand; }
QString ValenzInfo::controlCenterDiskUsagePath() const { return m_controlCenterDiskUsagePath; }
bool ValenzInfo::mprisAlwaysVisible() const { return m_mprisAlwaysVisible; }
double ValenzInfo::weatherLatitude() const { return m_weatherLatitude; }
double ValenzInfo::weatherLongitude() const { return m_weatherLongitude; }
QString ValenzInfo::weatherTemperatureUnit() const { return m_weatherTemperatureUnit; }
int ValenzInfo::weatherRefreshMinutes() const { return m_weatherRefreshMinutes; }

void ValenzInfo::setChanged() { Q_EMIT settingsChanged(); }

#define SET_BOUNDED_INTEGER(Name, Member, Minimum, Maximum) \
    void ValenzInfo::set##Name(int value) \
    { \
        value = qBound(Minimum, value, Maximum); \
        if (Member == value) \
            return; \
        Member = value; \
        setChanged(); \
    }

SET_BOUNDED_INTEGER(BarHeight, m_barHeight, 1, 100)
SET_BOUNDED_INTEGER(BarLayerSpacing, m_barLayerSpacing, 0, 64)
SET_BOUNDED_INTEGER(BarLayerSpacingTop, m_barLayerSpacingTop, 0, 64)
SET_BOUNDED_INTEGER(BarLayerSpacingBottom, m_barLayerSpacingBottom, 0, 64)
SET_BOUNDED_INTEGER(BarLayerSpacingLeft, m_barLayerSpacingLeft, 0, 64)
SET_BOUNDED_INTEGER(BarLayerSpacingRight, m_barLayerSpacingRight, 0, 64)

#undef SET_BOUNDED_INTEGER

void ValenzInfo::setScreenPlacement(const QString &value)
{
    const QString normalized = normalizedChoice(value, {QStringLiteral("active"), QStringLiteral("all")}, QStringLiteral("active"));
    if (m_screenPlacement == normalized)
        return;
    m_screenPlacement = normalized;
    setChanged();
}

void ValenzInfo::setControlCenterIconMode(const QString &value)
{
    const QString normalized = normalizedChoice(value, {QStringLiteral("system16"), QStringLiteral("nerd")}, QStringLiteral("system16"));
    if (m_controlCenterIconMode == normalized)
        return;
    m_controlCenterIconMode = normalized;
    setChanged();
}

void ValenzInfo::setControlCenterPowerCommand(const QString &value)
{
    const QString normalized = value.trimmed().isEmpty() ? QStringLiteral("wlogout") : value.trimmed();
    if (m_controlCenterPowerCommand == normalized)
        return;
    m_controlCenterPowerCommand = normalized;
    setChanged();
}

void ValenzInfo::setControlCenterSettingsCommand(const QString &value)
{
    const QString normalized = value.trimmed().isEmpty() ? QStringLiteral("systemsettings") : value.trimmed();
    if (m_controlCenterSettingsCommand == normalized)
        return;
    m_controlCenterSettingsCommand = normalized;
    setChanged();
}

void ValenzInfo::setControlCenterDiskUsagePath(const QString &value)
{
    QString normalized = value.trimmed();
    if (normalized.isEmpty())
        normalized = QStringLiteral("/");
    if (!normalized.startsWith(QLatin1Char('/')))
        normalized.prepend(QLatin1Char('/'));
    if (m_controlCenterDiskUsagePath == normalized)
        return;
    m_controlCenterDiskUsagePath = normalized;
    setChanged();
}

void ValenzInfo::setMprisAlwaysVisible(bool value)
{
    if (m_mprisAlwaysVisible == value)
        return;
    m_mprisAlwaysVisible = value;
    setChanged();
}

void ValenzInfo::setWeatherLatitude(double value)
{
    value = qBound(-90.0, value, 90.0);
    if (qFuzzyCompare(m_weatherLatitude, value))
        return;
    m_weatherLatitude = value;
    setChanged();
}

void ValenzInfo::setWeatherLongitude(double value)
{
    value = qBound(-180.0, value, 180.0);
    if (qFuzzyCompare(m_weatherLongitude, value))
        return;
    m_weatherLongitude = value;
    setChanged();
}

void ValenzInfo::setWeatherTemperatureUnit(const QString &value)
{
    const QString normalized = normalizedChoice(value, {QStringLiteral("celsius"), QStringLiteral("fahrenheit")}, QStringLiteral("celsius"));
    if (m_weatherTemperatureUnit == normalized)
        return;
    m_weatherTemperatureUnit = normalized;
    setChanged();
}

void ValenzInfo::setWeatherRefreshMinutes(int value)
{
    value = qBound(5, value, 180);
    if (m_weatherRefreshMinutes == value)
        return;
    m_weatherRefreshMinutes = value;
    setChanged();
}

void ValenzInfo::reload()
{
    load();
}

void ValenzInfo::load()
{
    const QSettings userSettings(m_configPath, QSettings::IniFormat);
    const QSettings distroSettings(QString::fromLatin1(distroConfigPath), QSettings::IniFormat);
    const auto value = [&userSettings, &distroSettings](const QString &key, const QVariant &fallback)
    {
        return settingValue(userSettings, distroSettings, key, fallback);
    };

    m_barHeight = qBound(1, value(QStringLiteral("Window/barHeight"), 56).toInt(), 100);
    m_barLayerSpacing = qBound(0, value(QStringLiteral("Window/barLayerSpacing"), 0).toInt(), 64);
    m_barLayerSpacingTop = qBound(0, value(QStringLiteral("Window/barLayerSpacingTop"), m_barLayerSpacing).toInt(), 64);
    m_barLayerSpacingBottom = qBound(0, value(QStringLiteral("Window/barLayerSpacingBottom"), m_barLayerSpacing).toInt(), 64);
    m_barLayerSpacingLeft = qBound(0, value(QStringLiteral("Window/barLayerSpacingLeft"), m_barLayerSpacing).toInt(), 64);
    m_barLayerSpacingRight = qBound(0, value(QStringLiteral("Window/barLayerSpacingRight"), m_barLayerSpacing).toInt(), 64);
    m_screenPlacement = normalizedChoice(value(QStringLiteral("Window/screenPlacement"), QStringLiteral("active")).toString(), {QStringLiteral("active"), QStringLiteral("all")}, QStringLiteral("active"));
    m_controlCenterIconMode = normalizedChoice(value(QStringLiteral("ControlCenter/iconMode"), QStringLiteral("system16")).toString(), {QStringLiteral("system16"), QStringLiteral("nerd")}, QStringLiteral("system16"));
    m_controlCenterPowerCommand = value(QStringLiteral("ControlCenter/powerCommand"), QStringLiteral("wlogout")).toString().trimmed();
    m_controlCenterSettingsCommand = value(QStringLiteral("ControlCenter/settingsCommand"), QStringLiteral("systemsettings")).toString().trimmed();
    m_controlCenterDiskUsagePath = value(QStringLiteral("ControlCenter/diskUsagePath"), QStringLiteral("/")).toString().trimmed();
    m_mprisAlwaysVisible = value(QStringLiteral("Mpris/alwaysVisible"), false).toBool();
    m_weatherLatitude = qBound(-90.0, value(QStringLiteral("Weather/latitude"), 40.7128).toDouble(), 90.0);
    m_weatherLongitude = qBound(-180.0, value(QStringLiteral("Weather/longitude"), -74.0060).toDouble(), 180.0);
    m_weatherTemperatureUnit = normalizedChoice(value(QStringLiteral("Weather/temperatureUnit"), QStringLiteral("celsius")).toString(), {QStringLiteral("celsius"), QStringLiteral("fahrenheit")}, QStringLiteral("celsius"));
    m_weatherRefreshMinutes = qBound(5, value(QStringLiteral("Weather/refreshMinutes"), 20).toInt(), 180);

    if (m_controlCenterPowerCommand.isEmpty())
        m_controlCenterPowerCommand = QStringLiteral("wlogout");
    if (m_controlCenterSettingsCommand.isEmpty())
        m_controlCenterSettingsCommand = QStringLiteral("systemsettings");
    if (m_controlCenterDiskUsagePath.isEmpty())
        m_controlCenterDiskUsagePath = QStringLiteral("/");

    setChanged();
}

bool ValenzInfo::save()
{
    QDir().mkpath(QDir::homePath() + QStringLiteral("/.config/valenz"));
    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("Window/barHeight"), m_barHeight);
    settings.setValue(QStringLiteral("Window/barLayerSpacing"), m_barLayerSpacing);
    settings.setValue(QStringLiteral("Window/barLayerSpacingTop"), m_barLayerSpacingTop);
    settings.setValue(QStringLiteral("Window/barLayerSpacingBottom"), m_barLayerSpacingBottom);
    settings.setValue(QStringLiteral("Window/barLayerSpacingLeft"), m_barLayerSpacingLeft);
    settings.setValue(QStringLiteral("Window/barLayerSpacingRight"), m_barLayerSpacingRight);
    settings.setValue(QStringLiteral("Window/screenPlacement"), m_screenPlacement);
    settings.setValue(QStringLiteral("ControlCenter/iconMode"), m_controlCenterIconMode);
    settings.setValue(QStringLiteral("ControlCenter/powerCommand"), m_controlCenterPowerCommand);
    settings.setValue(QStringLiteral("ControlCenter/settingsCommand"), m_controlCenterSettingsCommand);
    settings.setValue(QStringLiteral("ControlCenter/diskUsagePath"), m_controlCenterDiskUsagePath);
    settings.setValue(QStringLiteral("Mpris/alwaysVisible"), m_mprisAlwaysVisible);
    settings.setValue(QStringLiteral("Weather/latitude"), m_weatherLatitude);
    settings.setValue(QStringLiteral("Weather/longitude"), m_weatherLongitude);
    settings.setValue(QStringLiteral("Weather/temperatureUnit"), m_weatherTemperatureUnit);
    settings.setValue(QStringLiteral("Weather/refreshMinutes"), m_weatherRefreshMinutes);
    settings.sync();
    return settings.status() == QSettings::NoError;
}
