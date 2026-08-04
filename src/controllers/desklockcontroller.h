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
    Q_PROPERTY(QString iconMode READ iconMode WRITE setIconMode NOTIFY iconModeChanged)
    Q_PROPERTY(bool blurEnabled READ blurEnabled WRITE setBlurEnabled NOTIFY blurEnabledChanged)
    Q_PROPERTY(bool overlayEnabled READ overlayEnabled WRITE setOverlayEnabled NOTIFY overlayEnabledChanged)
    Q_PROPERTY(double overlayOpacity READ overlayOpacity WRITE setOverlayOpacity NOTIFY overlayOpacityChanged)
    Q_PROPERTY(QString timeFormat READ timeFormat WRITE setTimeFormat NOTIFY timeFormatChanged)
    Q_PROPERTY(QString dateFormat READ dateFormat WRITE setDateFormat NOTIFY dateFormatChanged)
    Q_PROPERTY(bool lowercaseDate READ lowercaseDate WRITE setLowercaseDate NOTIFY lowercaseDateChanged)
    Q_PROPERTY(bool showSystemMonitor READ showSystemMonitor WRITE setShowSystemMonitor NOTIFY showSystemMonitorChanged)
    Q_PROPERTY(bool showBattery READ showBattery WRITE setShowBattery NOTIFY showBatteryChanged)
    Q_PROPERTY(int batteryUpdateInterval READ batteryUpdateInterval WRITE setBatteryUpdateInterval NOTIFY batteryUpdateIntervalChanged)
    Q_PROPERTY(bool showMediaPlayer READ showMediaPlayer WRITE setShowMediaPlayer NOTIFY showMediaPlayerChanged)
    Q_PROPERTY(int systemMonitorUpdateInterval READ systemMonitorUpdateInterval WRITE setSystemMonitorUpdateInterval NOTIFY systemMonitorUpdateIntervalChanged)
    Q_PROPERTY(bool hideCursor READ hideCursor WRITE setHideCursor NOTIFY hideCursorChanged)
    Q_PROPERTY(int dimTimeout READ dimTimeout WRITE setDimTimeout NOTIFY dimTimeoutChanged)
    Q_PROPERTY(bool idleLockEnabled READ idleLockEnabled WRITE setIdleLockEnabled NOTIFY idleLockEnabledChanged)
    Q_PROPERTY(int idleLockTimeout READ idleLockTimeout WRITE setIdleLockTimeout NOTIFY idleLockTimeoutChanged)
    Q_PROPERTY(int dpmsTimeout READ dpmsTimeout WRITE setDpmsTimeout NOTIFY dpmsTimeoutChanged)
    Q_PROPERTY(int suspendTimeout READ suspendTimeout WRITE setSuspendTimeout NOTIFY suspendTimeoutChanged)

public:
    explicit DesklockController(QObject *parent = nullptr);

    QString configPath() const;
    QString wallpaperDirectory() const;
    QString wallpaperPath() const;
    QString avatarDirectory() const;
    QString avatarPath() const;
    QString iconMode() const;
    bool blurEnabled() const;
    bool overlayEnabled() const;
    double overlayOpacity() const;
    QString timeFormat() const;
    QString dateFormat() const;
    bool lowercaseDate() const;
    bool showSystemMonitor() const;
    bool showBattery() const;
    int batteryUpdateInterval() const;
    bool showMediaPlayer() const;
    int systemMonitorUpdateInterval() const;
    bool hideCursor() const;
    int dimTimeout() const;
    bool idleLockEnabled() const;
    int idleLockTimeout() const;
    int dpmsTimeout() const;
    int suspendTimeout() const;

    void setWallpaperPath(const QString &value);
    void setAvatarPath(const QString &value);
    void setIconMode(const QString &value);
    void setBlurEnabled(bool value);
    void setOverlayEnabled(bool value);
    void setOverlayOpacity(double value);
    void setTimeFormat(const QString &value);
    void setDateFormat(const QString &value);
    void setLowercaseDate(bool value);
    void setShowSystemMonitor(bool value);
    void setShowBattery(bool value);
    void setBatteryUpdateInterval(int value);
    void setShowMediaPlayer(bool value);
    void setSystemMonitorUpdateInterval(int value);
    void setHideCursor(bool value);
    void setDimTimeout(int value);
    void setIdleLockEnabled(bool value);
    void setIdleLockTimeout(int value);
    void setDpmsTimeout(int value);
    void setSuspendTimeout(int value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();

Q_SIGNALS:
    void wallpaperPathChanged();
    void avatarPathChanged();
    void iconModeChanged();
    void blurEnabledChanged();
    void overlayEnabledChanged();
    void overlayOpacityChanged();
    void timeFormatChanged();
    void dateFormatChanged();
    void lowercaseDateChanged();
    void showSystemMonitorChanged();
    void showBatteryChanged();
    void batteryUpdateIntervalChanged();
    void showMediaPlayerChanged();
    void systemMonitorUpdateIntervalChanged();
    void hideCursorChanged();
    void dimTimeoutChanged();
    void idleLockEnabledChanged();
    void idleLockTimeoutChanged();
    void dpmsTimeoutChanged();
    void suspendTimeoutChanged();
    void configurationChanged(const QString &key);

private:
    void load();
    void notifyDesklock(const QString &key);
    bool saveHypridleConfiguration() const;
    void loadHypridleConfiguration();
    static QString normalizeLocalPath(const QString &value);

    QString m_configPath;
    QString m_hypridleConfigPath;
    QString m_wallpaperPath;
    QString m_avatarPath;
    QString m_iconMode = QStringLiteral("system");
    bool m_blurEnabled = true;
    bool m_overlayEnabled = true;
    double m_overlayOpacity = 0.76;
    QString m_timeFormat = QStringLiteral("hh:mm");
    QString m_dateFormat = QStringLiteral("dddd, dd MMMM yyyy");
    bool m_lowercaseDate = false;
    bool m_showSystemMonitor = true;
    bool m_showBattery = true;
    int m_batteryUpdateInterval = 30000;
    bool m_showMediaPlayer = true;
    int m_systemMonitorUpdateInterval = 3000;
    bool m_hideCursor = true;
    int m_dimTimeout = 300;
    bool m_idleLockEnabled = true;
    int m_idleLockTimeout = 350;
    int m_dpmsTimeout = 500;
    int m_suspendTimeout = 650;
};
