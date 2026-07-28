// Copyright 2026 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desklockcontroller.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
#include <QVariant>
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
QString DesklockController::timeFormat() const { return m_timeFormat; }
QString DesklockController::dateFormat() const { return m_dateFormat; }
bool DesklockController::fadeAnimationsEnabled() const { return m_fadeAnimationsEnabled; }
int DesklockController::fadeInDuration() const { return m_fadeInDuration; }
int DesklockController::fadeOutDuration() const { return m_fadeOutDuration; }
bool DesklockController::showSystemMonitor() const { return m_showSystemMonitor; }
bool DesklockController::showBattery() const { return m_showBattery; }
bool DesklockController::showMediaPlayer() const { return m_showMediaPlayer; }
int DesklockController::idleLockTimeout() const { return m_idleLockTimeout; }
int DesklockController::gracePeriod() const { return m_gracePeriod; }
QString DesklockController::lastError() const { return m_lastError; }

QString DesklockController::normalizeLocalPath(const QString &value)
{
    QString path = value.trimmed();
    if (path.startsWith(QStringLiteral("file:")))
        path = QUrl(path).toLocalFile();

    return path.isEmpty() ? QString() : QFileInfo(path).absoluteFilePath();
}

void DesklockController::setLastError(const QString &error)
{
    if (m_lastError == error)
        return;

    m_lastError = error;
    Q_EMIT lastErrorChanged();
}

bool DesklockController::persist(const QString &key, const QVariant &value)
{
    const QFileInfo configInfo(m_configPath);
    if (!QDir().mkpath(configInfo.absolutePath()))
    {
        setLastError(tr("Could not create the Desklock configuration directory."));
        return false;
    }

    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue(key, value);
    settings.sync();
    if (settings.status() != QSettings::NoError)
    {
        setLastError(tr("Could not write %1.").arg(m_configPath));
        return false;
    }

    setLastError({});
    notifyDesklock(key);
    Q_EMIT configurationChanged(key);
    return true;
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
    persist(QStringLiteral("Appearance/BackgroundImage"), m_wallpaperPath);
}

void DesklockController::setTimeFormat(const QString &value)
{
    const QString normalized = value.trimmed().isEmpty() ? QStringLiteral("hh:mm") : value.trimmed();
    if (m_timeFormat == normalized)
        return;

    m_timeFormat = normalized;
    Q_EMIT timeFormatChanged();
    persist(QStringLiteral("Clock/TimeFormat"), m_timeFormat);
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
    persist(QStringLiteral("Clock/DateFormat"), m_dateFormat);
}

void DesklockController::setFadeAnimationsEnabled(bool value)
{
    if (m_fadeAnimationsEnabled == value)
        return;

    m_fadeAnimationsEnabled = value;
    Q_EMIT fadeAnimationsEnabledChanged();
    persist(QStringLiteral("Behavior/FadeAnimationsEnabled"), value);
}

#define SET_BOUNDED_SETTING(Method, Member, Signal, Minimum, Maximum, Key) \
    void DesklockController::Method(int value) \
    { \
        value = qBound(Minimum, value, Maximum); \
        if (Member == value) \
            return; \
        Member = value; \
        Q_EMIT Signal(); \
        persist(QStringLiteral(Key), value); \
    }

SET_BOUNDED_SETTING(setFadeInDuration, m_fadeInDuration, fadeInDurationChanged, 0, 5000, "Behavior/FadeInDuration")
SET_BOUNDED_SETTING(setFadeOutDuration, m_fadeOutDuration, fadeOutDurationChanged, 0, 5000, "Behavior/FadeOutDuration")
SET_BOUNDED_SETTING(setIdleLockTimeout, m_idleLockTimeout, idleLockTimeoutChanged, 0, 86400, "Behavior/IdleLockTimeout")
SET_BOUNDED_SETTING(setGracePeriod, m_gracePeriod, gracePeriodChanged, 0, 3600, "Behavior/GracePeriod")

#undef SET_BOUNDED_SETTING

#define SET_BOOLEAN_SETTING(Method, Member, Signal, Key) \
    void DesklockController::Method(bool value) \
    { \
        if (Member == value) \
            return; \
        Member = value; \
        Q_EMIT Signal(); \
        persist(QStringLiteral(Key), value); \
    }

SET_BOOLEAN_SETTING(setShowSystemMonitor, m_showSystemMonitor, showSystemMonitorChanged, "SystemMonitor/Enabled")
SET_BOOLEAN_SETTING(setShowBattery, m_showBattery, showBatteryChanged, "Battery/Enabled")
SET_BOOLEAN_SETTING(setShowMediaPlayer, m_showMediaPlayer, showMediaPlayerChanged, "Media/Enabled")

#undef SET_BOOLEAN_SETTING

void DesklockController::reload()
{
    load();
}

void DesklockController::load()
{
    QSettings settings(m_configPath, QSettings::IniFormat);

    m_wallpaperPath = normalizeLocalPath(settings.value(
        QStringLiteral("Appearance/BackgroundImage"),
        QString::fromLatin1(defaultWallpaper)).toString());
    m_timeFormat = settings.value(QStringLiteral("Clock/TimeFormat"), QStringLiteral("hh:mm")).toString();
    m_dateFormat = settings.value(
        QStringLiteral("Clock/DateFormat"),
        QStringLiteral("dddd, dd MMMM yyyy")).toString();
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
    m_showMediaPlayer = settings.value(QStringLiteral("Media/Enabled"), true).toBool();
    m_idleLockTimeout = qBound(0, settings.value(
        QStringLiteral("Behavior/IdleLockTimeout"),
        300).toInt(), 86400);
    m_gracePeriod = qBound(0, settings.value(
        QStringLiteral("Behavior/GracePeriod"),
        0).toInt(), 3600);

    setLastError({});
    Q_EMIT wallpaperPathChanged();
    Q_EMIT timeFormatChanged();
    Q_EMIT dateFormatChanged();
    Q_EMIT fadeAnimationsEnabledChanged();
    Q_EMIT fadeInDurationChanged();
    Q_EMIT fadeOutDurationChanged();
    Q_EMIT showSystemMonitorChanged();
    Q_EMIT showBatteryChanged();
    Q_EMIT showMediaPlayerChanged();
    Q_EMIT idleLockTimeoutChanged();
    Q_EMIT gracePeriodChanged();
}
