#include "backgroundmanager.h"

#include "code/settingsstore.h"

#include <QDebug>

BackgroundManager::BackgroundManager(QObject *parent) : QObject(parent)
  ,m_settings(new SettingsStore(this))
  ,m_backgroundInterface("org.mauiman.Manager",
                         "/Background",
                         "org.mauiman.Background",
                         QDBusConnection::sessionBus(), this)
{
    qDebug( " INIT BACKGORUND MANAGER");

    m_settings->beginModule("Background");
    m_wallpaperSource = m_settings->load("Wallpaper", m_wallpaperSource).toString();
    m_dimWallpaper = m_settings->load("DimWallpaper", m_dimWallpaper).toBool();
    m_showWallpaper = m_settings->load("ShowWallpaper", m_showWallpaper).toBool();
    m_adaptiveColorScheme = m_settings->load("AdaptiveColorScheme", m_adaptiveColorScheme).toBool();
    m_fitWallpaper = m_settings->load("FitWallpaper", m_fitWallpaper).toBool();
    m_solidColor = m_settings->load("SolidColor", m_solidColor).toString();
    m_settings->endModule();

    if (m_backgroundInterface.isValid())
    {
        connect(&m_backgroundInterface, SIGNAL(wallpaperSourceChanged(QString)), this, SLOT(onWallpaperChanged(QString)));
        connect(&m_backgroundInterface, SIGNAL(solidColorChanged(QString)), this, SLOT(onSolidColorChanged(QString)));

    }
}

QString BackgroundManager::wallpaperSource() const
{
    return m_wallpaperSource;
}

bool BackgroundManager::dimWallpaper() const
{
    return m_dimWallpaper;
}

bool BackgroundManager::adaptiveColorScheme() const
{
    return m_adaptiveColorScheme;
}

bool BackgroundManager::fitWallpaper() const
{
    return m_fitWallpaper;
}

QString BackgroundManager::solidColor() const
{
    return m_solidColor;
}

bool BackgroundManager::showWallpaper() const
{
    return m_showWallpaper;
}

void BackgroundManager::setWallpaperSource(QString wallpaperSource)
{
    if (m_wallpaperSource == wallpaperSource)
        return;

    m_wallpaperSource = wallpaperSource;
    sync("setWallpaperSource", m_wallpaperSource);
    emit wallpaperSourceChanged(m_wallpaperSource);
}

void BackgroundManager::setDimWallpaper(bool dimWallpaper)
{
    if (m_dimWallpaper == dimWallpaper)
        return;

    m_dimWallpaper = dimWallpaper;
    sync("setDimWallpaper", m_dimWallpaper);
    emit dimWallpaperChanged(m_dimWallpaper);
}

void BackgroundManager::setAdaptiveColorScheme(bool adaptiveColorScheme)
{
    if (m_adaptiveColorScheme == adaptiveColorScheme)
        return;

    m_adaptiveColorScheme = adaptiveColorScheme;
    sync("setAdaptiveColorScheme", m_adaptiveColorScheme);
    emit adaptiveColorSchemeChanged(m_adaptiveColorScheme);
}

void BackgroundManager::setFitWallpaper(bool fitWallpaper)
{
    if (m_fitWallpaper == fitWallpaper)
        return;

    m_fitWallpaper = fitWallpaper;
    sync("setFitWallpaper", m_fitWallpaper);
    emit fitWallpaperChanged(m_fitWallpaper);
}

void BackgroundManager::setSolidColor(QString solidColor)
{
    if (m_solidColor == solidColor)
        return;

    m_solidColor = solidColor;
    sync("setSolidColor", m_solidColor);
    emit solidColorChanged(m_solidColor);
}

void BackgroundManager::setShowWallpaper(bool showWallpaper)
{
    if (m_showWallpaper == showWallpaper)
        return;

    m_showWallpaper = showWallpaper;
    sync("setShowWallpaper", m_showWallpaper);
    emit showWallpaperChanged(m_showWallpaper);
}

QString BackgroundManager::wallpaperSourceDir() const
{
    return m_wallpaperSourceDir;
}

void BackgroundManager::setWallpaperSourceDir(QString wallpaperSourceDir)
{
    if (m_wallpaperSourceDir == wallpaperSourceDir)
        return;

    m_wallpaperSourceDir = wallpaperSourceDir;
    emit wallpaperSourceDirChanged(m_wallpaperSourceDir);
}

void BackgroundManager::onWallpaperChanged(const QString &wallpaperSource)
{
    if (m_wallpaperSource == wallpaperSource)
        return;

    m_wallpaperSource = wallpaperSource;
    emit wallpaperSourceChanged(m_wallpaperSource);
}

void BackgroundManager::onSolidColorChanged(const QString &solidColor)
{
    if (m_solidColor == solidColor)
        return;

    m_solidColor = solidColor;
    emit solidColorChanged(m_solidColor);
}

void BackgroundManager::sync(const QString &key, const QVariant &value)
{
    if (m_backgroundInterface.isValid())
    {
        m_backgroundInterface.call(key, value);
    }
}
