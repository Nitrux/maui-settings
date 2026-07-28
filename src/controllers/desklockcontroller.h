// Copyright 2026 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QVariant>

class DesklockController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(QString wallpaperDirectory READ wallpaperDirectory NOTIFY wallpaperPathChanged)
    Q_PROPERTY(QString wallpaperPath READ wallpaperPath WRITE setWallpaperPath NOTIFY wallpaperPathChanged)
    Q_PROPERTY(QString timeFormat READ timeFormat WRITE setTimeFormat NOTIFY timeFormatChanged)
    Q_PROPERTY(QString dateFormat READ dateFormat WRITE setDateFormat NOTIFY dateFormatChanged)
    Q_PROPERTY(bool fadeAnimationsEnabled READ fadeAnimationsEnabled WRITE setFadeAnimationsEnabled NOTIFY fadeAnimationsEnabledChanged)
    Q_PROPERTY(int fadeInDuration READ fadeInDuration WRITE setFadeInDuration NOTIFY fadeInDurationChanged)
    Q_PROPERTY(int fadeOutDuration READ fadeOutDuration WRITE setFadeOutDuration NOTIFY fadeOutDurationChanged)
    Q_PROPERTY(bool showSystemMonitor READ showSystemMonitor WRITE setShowSystemMonitor NOTIFY showSystemMonitorChanged)
    Q_PROPERTY(bool showBattery READ showBattery WRITE setShowBattery NOTIFY showBatteryChanged)
    Q_PROPERTY(bool showMediaPlayer READ showMediaPlayer WRITE setShowMediaPlayer NOTIFY showMediaPlayerChanged)
    Q_PROPERTY(int idleLockTimeout READ idleLockTimeout WRITE setIdleLockTimeout NOTIFY idleLockTimeoutChanged)
    Q_PROPERTY(int gracePeriod READ gracePeriod WRITE setGracePeriod NOTIFY gracePeriodChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit DesklockController(QObject *parent = nullptr);

    QString configPath() const;
    QString wallpaperDirectory() const;
    QString wallpaperPath() const;
    QString timeFormat() const;
    QString dateFormat() const;
    bool fadeAnimationsEnabled() const;
    int fadeInDuration() const;
    int fadeOutDuration() const;
    bool showSystemMonitor() const;
    bool showBattery() const;
    bool showMediaPlayer() const;
    int idleLockTimeout() const;
    int gracePeriod() const;
    QString lastError() const;

    void setWallpaperPath(const QString &value);
    void setTimeFormat(const QString &value);
    void setDateFormat(const QString &value);
    void setFadeAnimationsEnabled(bool value);
    void setFadeInDuration(int value);
    void setFadeOutDuration(int value);
    void setShowSystemMonitor(bool value);
    void setShowBattery(bool value);
    void setShowMediaPlayer(bool value);
    void setIdleLockTimeout(int value);
    void setGracePeriod(int value);

    Q_INVOKABLE void reload();

Q_SIGNALS:
    void wallpaperPathChanged();
    void timeFormatChanged();
    void dateFormatChanged();
    void fadeAnimationsEnabledChanged();
    void fadeInDurationChanged();
    void fadeOutDurationChanged();
    void showSystemMonitorChanged();
    void showBatteryChanged();
    void showMediaPlayerChanged();
    void idleLockTimeoutChanged();
    void gracePeriodChanged();
    void lastErrorChanged();
    void configurationChanged(const QString &key);

private:
    void load();
    bool persist(const QString &key, const QVariant &value);
    void notifyDesklock(const QString &key);
    void setLastError(const QString &error);
    static QString normalizeLocalPath(const QString &value);

    QString m_configPath;
    QString m_wallpaperPath;
    QString m_timeFormat = QStringLiteral("hh:mm");
    QString m_dateFormat = QStringLiteral("dddd, dd MMMM yyyy");
    bool m_fadeAnimationsEnabled = true;
    int m_fadeInDuration = 350;
    int m_fadeOutDuration = 250;
    bool m_showSystemMonitor = true;
    bool m_showBattery = true;
    bool m_showMediaPlayer = true;
    int m_idleLockTimeout = 300;
    int m_gracePeriod = 0;
    QString m_lastError;
};
