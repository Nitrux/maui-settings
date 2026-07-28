// Copyright 2026 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desklockcontroller.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
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
}

DesklockController::DesklockController(QObject *parent)
    : QObject(parent)
    , m_configPath(desklockConfigPath())
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
int DesklockController::backgroundBlurRadius() const { return m_backgroundBlurRadius; }
double DesklockController::backgroundOverlayOpacity() const { return m_backgroundOverlayOpacity; }
QString DesklockController::timeFormat() const { return m_timeFormat; }
QString DesklockController::dateFormat() const { return m_dateFormat; }
bool DesklockController::lowercaseDate() const { return m_lowercaseDate; }
bool DesklockController::fadeAnimationsEnabled() const { return m_fadeAnimationsEnabled; }
int DesklockController::fadeInDuration() const { return m_fadeInDuration; }
int DesklockController::fadeOutDuration() const { return m_fadeOutDuration; }
bool DesklockController::showSystemMonitor() const { return m_showSystemMonitor; }
bool DesklockController::showBattery() const { return m_showBattery; }
int DesklockController::batteryUpdateInterval() const { return m_batteryUpdateInterval; }
bool DesklockController::showMediaPlayer() const { return m_showMediaPlayer; }
int DesklockController::systemMonitorUpdateInterval() const { return m_systemMonitorUpdateInterval; }
bool DesklockController::hideCursor() const { return m_hideCursor; }
int DesklockController::idleLockTimeout() const { return m_idleLockTimeout; }
int DesklockController::gracePeriod() const { return m_gracePeriod; }

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

void DesklockController::setBackgroundOverlayOpacity(double value)
{
    value = qBound(0.0, value, 1.0);
    if (qFuzzyCompare(m_backgroundOverlayOpacity, value))
        return;

    m_backgroundOverlayOpacity = value;
    Q_EMIT backgroundOverlayOpacityChanged();
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

void DesklockController::setFadeAnimationsEnabled(bool value)
{
    if (m_fadeAnimationsEnabled == value)
        return;

    m_fadeAnimationsEnabled = value;
    Q_EMIT fadeAnimationsEnabledChanged();
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

SET_BOUNDED_SETTING(setFadeInDuration, m_fadeInDuration, fadeInDurationChanged, 0, 5000)
SET_BOUNDED_SETTING(setFadeOutDuration, m_fadeOutDuration, fadeOutDurationChanged, 0, 5000)
SET_BOUNDED_SETTING(setBackgroundBlurRadius, m_backgroundBlurRadius, backgroundBlurRadiusChanged, 0, 128)
SET_BOUNDED_SETTING(setIdleLockTimeout, m_idleLockTimeout, idleLockTimeoutChanged, 0, 86400)
SET_BOUNDED_SETTING(setGracePeriod, m_gracePeriod, gracePeriodChanged, 0, 3600)
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

    const auto writeSettings = [this](bool atomicSync)
    {
        QSettings settings(m_configPath, QSettings::IniFormat);
        settings.setAtomicSyncRequired(atomicSync);
        settings.setValue(QStringLiteral("Appearance/BackgroundImage"), m_wallpaperPath);
        settings.setValue(QStringLiteral("Appearance/AvatarImage"), m_avatarPath);
        settings.setValue(QStringLiteral("Appearance/BackgroundBlurRadius"), m_backgroundBlurRadius);
        settings.setValue(QStringLiteral("Appearance/BackgroundOverlayOpacity"), m_backgroundOverlayOpacity);
        settings.setValue(QStringLiteral("Clock/TimeFormat"), m_timeFormat);
        settings.setValue(QStringLiteral("Clock/DateFormat"), m_dateFormat);
        settings.setValue(QStringLiteral("Clock/LowercaseDate"), m_lowercaseDate);
        settings.setValue(QStringLiteral("Behavior/FadeAnimationsEnabled"), m_fadeAnimationsEnabled);
        settings.setValue(QStringLiteral("Behavior/FadeInDuration"), m_fadeInDuration);
        settings.setValue(QStringLiteral("Behavior/FadeOutDuration"), m_fadeOutDuration);
        settings.setValue(QStringLiteral("SystemMonitor/Enabled"), m_showSystemMonitor);
        settings.setValue(QStringLiteral("Battery/Enabled"), m_showBattery);
        settings.setValue(QStringLiteral("Battery/UpdateInterval"), m_batteryUpdateInterval);
        settings.setValue(QStringLiteral("Media/Enabled"), m_showMediaPlayer);
        settings.setValue(QStringLiteral("SystemMonitor/UpdateInterval"), m_systemMonitorUpdateInterval);
        settings.setValue(QStringLiteral("Behavior/HideCursor"), m_hideCursor);
        settings.setValue(QStringLiteral("Behavior/IdleLockTimeout"), m_idleLockTimeout);
        settings.setValue(QStringLiteral("Behavior/GracePeriod"), m_gracePeriod);
        settings.sync();
        return settings.status();
    };

    QSettings::Status status = writeSettings(true);
    if (status == QSettings::AccessError && configInfo.exists() && configInfo.isWritable())
        status = writeSettings(false);

    if (status != QSettings::NoError)
    {
        qWarning() << "Could not write Desklock configuration" << m_configPath
                   << "QSettings status" << static_cast<int>(status);
        return false;
    }

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
    m_backgroundBlurRadius = qBound(0, settings.value(
        QStringLiteral("Appearance/BackgroundBlurRadius"), 64).toInt(), 128);
    m_backgroundOverlayOpacity = qBound(0.0, settings.value(
        QStringLiteral("Appearance/BackgroundOverlayOpacity"), 0.76).toDouble(), 1.0);
    m_timeFormat = settings.value(QStringLiteral("Clock/TimeFormat"), QStringLiteral("hh:mm")).toString();
    m_dateFormat = settings.value(
        QStringLiteral("Clock/DateFormat"),
        QStringLiteral("dddd, dd MMMM yyyy")).toString();
    m_lowercaseDate = settings.value(QStringLiteral("Clock/LowercaseDate"), false).toBool();
    m_fadeAnimationsEnabled = settings.value(
        QStringLiteral("Behavior/FadeAnimationsEnabled"),
        true).toBool();
    m_fadeInDuration = qBound(0, settings.value(
        QStringLiteral("Behavior/FadeInDuration"),
        350).toInt(), 5000);
    m_fadeOutDuration = qBound(0, settings.value(
        QStringLiteral("Behavior/FadeOutDuration"),
        250).toInt(), 5000);
    m_showSystemMonitor = settings.value(QStringLiteral("SystemMonitor/Enabled"), true).toBool();
    m_showBattery = settings.value(QStringLiteral("Battery/Enabled"), true).toBool();
    m_batteryUpdateInterval = qBound(1000, settings.value(
        QStringLiteral("Battery/UpdateInterval"), 30000).toInt(), 3600000);
    m_showMediaPlayer = settings.value(QStringLiteral("Media/Enabled"), true).toBool();
    m_systemMonitorUpdateInterval = qBound(1000, settings.value(
        QStringLiteral("SystemMonitor/UpdateInterval"), 3000).toInt(), 3600000);
    m_hideCursor = settings.value(QStringLiteral("Behavior/HideCursor"), true).toBool();
    m_idleLockTimeout = qBound(0, settings.value(
        QStringLiteral("Behavior/IdleLockTimeout"),
        300).toInt(), 86400);
    m_gracePeriod = qBound(0, settings.value(
        QStringLiteral("Behavior/GracePeriod"),
        0).toInt(), 3600);

    Q_EMIT wallpaperPathChanged();
    Q_EMIT avatarPathChanged();
    Q_EMIT backgroundBlurRadiusChanged();
    Q_EMIT backgroundOverlayOpacityChanged();
    Q_EMIT timeFormatChanged();
    Q_EMIT dateFormatChanged();
    Q_EMIT lowercaseDateChanged();
    Q_EMIT fadeAnimationsEnabledChanged();
    Q_EMIT fadeInDurationChanged();
    Q_EMIT fadeOutDurationChanged();
    Q_EMIT showSystemMonitorChanged();
    Q_EMIT showBatteryChanged();
    Q_EMIT batteryUpdateIntervalChanged();
    Q_EMIT showMediaPlayerChanged();
    Q_EMIT systemMonitorUpdateIntervalChanged();
    Q_EMIT hideCursorChanged();
    Q_EMIT idleLockTimeoutChanged();
    Q_EMIT gracePeriodChanged();
}
