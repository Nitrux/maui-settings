#pragma once

#include <QObject>
#include <QString>
#include <QQmlEngine>
#include <QDBusInterface>

class SettingsStore;
class BackgroundManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString wallpaperSource READ wallpaperSource WRITE setWallpaperSource NOTIFY wallpaperSourceChanged)
    Q_PROPERTY(bool dimWallpaper READ dimWallpaper WRITE setDimWallpaper NOTIFY dimWallpaperChanged)
    Q_PROPERTY(bool adaptiveColorScheme READ adaptiveColorScheme WRITE setAdaptiveColorScheme NOTIFY adaptiveColorSchemeChanged)
    Q_PROPERTY(bool fitWallpaper READ fitWallpaper WRITE setFitWallpaper NOTIFY fitWallpaperChanged)
    Q_PROPERTY(QString solidColor READ solidColor WRITE setSolidColor NOTIFY solidColorChanged)
    Q_PROPERTY(bool showWallpaper READ showWallpaper WRITE setShowWallpaper NOTIFY showWallpaperChanged)

    Q_PROPERTY(QString wallpaperSourceDir READ wallpaperSourceDir WRITE setWallpaperSourceDir NOTIFY wallpaperSourceDirChanged)


public:
    explicit BackgroundManager(QObject * parent = nullptr);

    QString wallpaperSource() const;

    bool dimWallpaper() const;

    bool adaptiveColorScheme() const;

    bool fitWallpaper() const;

    QString solidColor() const;

    bool showWallpaper() const;

    void setWallpaperSource(QString wallpaperSource);

    void setDimWallpaper(bool dimWallpaper);

    void setAdaptiveColorScheme(bool adaptiveColorScheme);

    void setFitWallpaper(bool fitWallpaper);

    void setSolidColor(QString solidColor);

    void setShowWallpaper(bool showWallpaper);

    QString wallpaperSourceDir() const;
    void setWallpaperSourceDir(QString wallpaperSourceDir);

private slots:
    void onWallpaperChanged(const QString &wallpaperSource);
    void onSolidColorChanged(const QString &solidColor);

signals:
    void wallpaperSourceChanged(QString wallpaperSource);

    void dimWallpaperChanged(bool dimWallpaper);

    void adaptiveColorSchemeChanged(bool adaptiveColorScheme);

    void fitWallpaperChanged(bool fitWallpaper);

    void solidColorChanged(QString solidColor);

    void showWallpaperChanged(bool showWallpaper);

    void wallpaperSourceDirChanged(QString wallpaperSourceDir);

private:
    SettingsStore *m_settings;
    QDBusInterface m_backgroundInterface;

    QString m_wallpaperSource = "file:///home/camilo/Pictures/Wallpapers/new_wall (2).png";
    bool m_dimWallpaper = false;
    bool m_adaptiveColorScheme = true;
    bool m_fitWallpaper = false; //false is to fill, true to fit
    QString m_solidColor = "#333";
    bool m_showWallpaper = true;

    QString m_wallpaperSourceDir = "file:///usr/share/wallpapers/Cask";

    void sync(const QString &key, const QVariant &value);
};

