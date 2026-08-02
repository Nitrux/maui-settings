#pragma once

#include <QObject>
#include <QString>

class DesklockController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(QString wallpaperDirectory READ wallpaperDirectory NOTIFY wallpaperPathChanged)
    Q_PROPERTY(QString wallpaperPath READ wallpaperPath WRITE setWallpaperPath NOTIFY wallpaperPathChanged)
    Q_PROPERTY(QString avatarDirectory READ avatarDirectory NOTIFY avatarPathChanged)
    Q_PROPERTY(QString avatarPath READ avatarPath WRITE setAvatarPath NOTIFY avatarPathChanged)
    Q_PROPERTY(int backgroundBlurRadius READ backgroundBlurRadius WRITE setBackgroundBlurRadius NOTIFY backgroundBlurRadiusChanged)
    Q_PROPERTY(double backgroundOverlayOpacity READ backgroundOverlayOpacity WRITE setBackgroundOverlayOpacity NOTIFY backgroundOverlayOpacityChanged)
    Q_PROPERTY(QString timeFormat READ timeFormat WRITE setTimeFormat NOTIFY timeFormatChanged)
    Q_PROPERTY(QString dateFormat READ dateFormat WRITE setDateFormat NOTIFY dateFormatChanged)
    Q_PROPERTY(bool lowercaseDate READ lowercaseDate WRITE setLowercaseDate NOTIFY lowercaseDateChanged)
    Q_PROPERTY(bool fadeAnimationsEnabled READ fadeAnimationsEnabled WRITE setFadeAnimationsEnabled NOTIFY fadeAnimationsEnabledChanged)
    Q_PROPERTY(int fadeInDuration READ fadeInDuration WRITE setFadeInDuration NOTIFY fadeInDurationChanged)
    Q_PROPERTY(int fadeOutDuration READ fadeOutDuration WRITE setFadeOutDuration NOTIFY fadeOutDurationChanged)
    Q_PROPERTY(bool showSystemMonitor READ showSystemMonitor WRITE setShowSystemMonitor NOTIFY showSystemMonitorChanged)
    Q_PROPERTY(bool showBattery READ showBattery WRITE setShowBattery NOTIFY showBatteryChanged)
    Q_PROPERTY(int batteryUpdateInterval READ batteryUpdateInterval WRITE setBatteryUpdateInterval NOTIFY batteryUpdateIntervalChanged)
    Q_PROPERTY(bool showMediaPlayer READ showMediaPlayer WRITE setShowMediaPlayer NOTIFY showMediaPlayerChanged)
    Q_PROPERTY(int systemMonitorUpdateInterval READ systemMonitorUpdateInterval WRITE setSystemMonitorUpdateInterval NOTIFY systemMonitorUpdateIntervalChanged)
    Q_PROPERTY(bool hideCursor READ hideCursor WRITE setHideCursor NOTIFY hideCursorChanged)
    Q_PROPERTY(int idleLockTimeout READ idleLockTimeout WRITE setIdleLockTimeout NOTIFY idleLockTimeoutChanged)
    Q_PROPERTY(int gracePeriod READ gracePeriod WRITE setGracePeriod NOTIFY gracePeriodChanged)

public:
    explicit DesklockController(QObject *parent = nullptr);

    QString configPath() const;
    QString wallpaperDirectory() const;
    QString wallpaperPath() const;
    QString avatarDirectory() const;
    QString avatarPath() const;
    int backgroundBlurRadius() const;
    double backgroundOverlayOpacity() const;
    QString timeFormat() const;
    QString dateFormat() const;
    bool lowercaseDate() const;
    bool fadeAnimationsEnabled() const;
    int fadeInDuration() const;
    int fadeOutDuration() const;
    bool showSystemMonitor() const;
    bool showBattery() const;
    int batteryUpdateInterval() const;
    bool showMediaPlayer() const;
    int systemMonitorUpdateInterval() const;
    bool hideCursor() const;
    int idleLockTimeout() const;
    int gracePeriod() const;

    void setWallpaperPath(const QString &value);
    void setAvatarPath(const QString &value);
    void setBackgroundBlurRadius(int value);
    void setBackgroundOverlayOpacity(double value);
    void setTimeFormat(const QString &value);
    void setDateFormat(const QString &value);
    void setLowercaseDate(bool value);
    void setFadeAnimationsEnabled(bool value);
    void setFadeInDuration(int value);
    void setFadeOutDuration(int value);
    void setShowSystemMonitor(bool value);
    void setShowBattery(bool value);
    void setBatteryUpdateInterval(int value);
    void setShowMediaPlayer(bool value);
    void setSystemMonitorUpdateInterval(int value);
    void setHideCursor(bool value);
    void setIdleLockTimeout(int value);
    void setGracePeriod(int value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();

Q_SIGNALS:
    void wallpaperPathChanged();
    void avatarPathChanged();
    void backgroundBlurRadiusChanged();
    void backgroundOverlayOpacityChanged();
    void timeFormatChanged();
    void dateFormatChanged();
    void lowercaseDateChanged();
    void fadeAnimationsEnabledChanged();
    void fadeInDurationChanged();
    void fadeOutDurationChanged();
    void showSystemMonitorChanged();
    void showBatteryChanged();
    void batteryUpdateIntervalChanged();
    void showMediaPlayerChanged();
    void systemMonitorUpdateIntervalChanged();
    void hideCursorChanged();
    void idleLockTimeoutChanged();
    void gracePeriodChanged();
    void configurationChanged(const QString &key);

private:
    void load();
    void notifyDesklock(const QString &key);
    static QString normalizeLocalPath(const QString &value);

    QString m_configPath;
    QString m_wallpaperPath;
    QString m_avatarPath;
    int m_backgroundBlurRadius = 64;
    double m_backgroundOverlayOpacity = 0.76;
    QString m_timeFormat = QStringLiteral("hh:mm");
    QString m_dateFormat = QStringLiteral("dddd, dd MMMM yyyy");
    bool m_lowercaseDate = false;
    bool m_fadeAnimationsEnabled = true;
    int m_fadeInDuration = 350;
    int m_fadeOutDuration = 250;
    bool m_showSystemMonitor = true;
    bool m_showBattery = true;
    int m_batteryUpdateInterval = 30000;
    bool m_showMediaPlayer = true;
    int m_systemMonitorUpdateInterval = 3000;
    bool m_hideCursor = true;
    int m_idleLockTimeout = 300;
    int m_gracePeriod = 0;
};
