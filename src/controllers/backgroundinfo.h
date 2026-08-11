#pragma once

#include <QObject>
#include <QString>

class BackgroundInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(QString wallpaperDirectory READ wallpaperDirectory NOTIFY settingsChanged)
    Q_PROPERTY(QString wallpaperMonitor READ wallpaperMonitor WRITE setWallpaperMonitor NOTIFY settingsChanged)
    Q_PROPERTY(QString wallpaperPath READ wallpaperPath WRITE setWallpaperPath NOTIFY settingsChanged)
    Q_PROPERTY(QString wallpaperFitMode READ wallpaperFitMode WRITE setWallpaperFitMode NOTIFY settingsChanged)
    Q_PROPERTY(int wallpaperTimeout READ wallpaperTimeout WRITE setWallpaperTimeout NOTIFY settingsChanged)
    Q_PROPERTY(QString wallpaperOrder READ wallpaperOrder WRITE setWallpaperOrder NOTIFY settingsChanged)
    Q_PROPERTY(bool wallpaperRecursive READ wallpaperRecursive WRITE setWallpaperRecursive NOTIFY settingsChanged)
    Q_PROPERTY(bool splashEnabled READ splashEnabled WRITE setSplashEnabled NOTIFY settingsChanged)
    Q_PROPERTY(int splashOffset READ splashOffset WRITE setSplashOffset NOTIFY settingsChanged)
    Q_PROPERTY(double splashOpacity READ splashOpacity WRITE setSplashOpacity NOTIFY settingsChanged)
    Q_PROPERTY(bool ipcEnabled READ ipcEnabled WRITE setIpcEnabled NOTIFY settingsChanged)

public:
    explicit BackgroundInfo(QObject *parent = nullptr);

    QString configPath() const;
    bool available() const;
    QString wallpaperDirectory() const;
    QString wallpaperMonitor() const;
    QString wallpaperPath() const;
    QString wallpaperFitMode() const;
    int wallpaperTimeout() const;
    QString wallpaperOrder() const;
    bool wallpaperRecursive() const;
    bool splashEnabled() const;
    int splashOffset() const;
    double splashOpacity() const;
    bool ipcEnabled() const;

    void setWallpaperMonitor(const QString &value);
    void setWallpaperPath(const QString &value);
    void setWallpaperFitMode(const QString &value);
    void setWallpaperTimeout(int value);
    void setWallpaperOrder(const QString &value);
    void setWallpaperRecursive(bool value);
    void setSplashEnabled(bool value);
    void setSplashOffset(int value);
    void setSplashOpacity(double value);
    void setIpcEnabled(bool value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    static QString homeConfigPath();
    static QString normalizePath(const QString &value);
    static QString unquoteValue(const QString &value);
    static bool parseBool(const QString &value);
    void setChanged();

private:
    QString m_configPath;
    bool m_available = false;
    QString m_wallpaperMonitor;
    QString m_wallpaperPath;
    QString m_wallpaperFitMode = QStringLiteral("cover");
    int m_wallpaperTimeout = 0;
    QString m_wallpaperOrder = QStringLiteral("default");
    bool m_wallpaperRecursive = false;
    bool m_splashEnabled = false;
    int m_splashOffset = 20;
    double m_splashOpacity = 0.8;
    bool m_ipcEnabled = true;
};
