#include "background.h"
#include "backgroundadaptor.h"
#include <QDBusInterface>

#include "code/settingsstore.h"

#include <QDebug>

Background::Background(QObject *parent) : QObject(parent)
  ,m_settings(new SettingsStore(this))
{
    qDebug( " INIT BACKGORUND MANAGER");
    new BackgroundAdaptor(this);
    if(!QDBusConnection::sessionBus().registerObject(QStringLiteral("/Background"), this))
    {
        qDebug() << "FAILED TO REGISTER BACKGROUND DBUS OBJECT";
        return;
    }

    m_settings->beginModule("Background");
    m_wallpaperSource = m_settings->load("Wallpaper", "file:///home/camilo/Pictures/Wallpapers/new_wall (2).png").toString();
}

QString Background::wallpaperSource() const
{
    return m_wallpaperSource;
}

bool Background::dimWallpaper() const
{
    return m_dimWallpaper;
}

bool Background::adaptiveColorScheme() const
{
    return m_adaptiveColorScheme;
}

bool Background::fitWallpaper() const
{
    return m_fitWallpaper;
}

QString Background::solidColor() const
{
    return m_solidColor;
}

bool Background::showWallpaper() const
{
    return m_showWallpaper;
}

void Background::setWallpaperSource(QString wallpaperSource)
{
    if (m_wallpaperSource == wallpaperSource)
        return;

    m_wallpaperSource = wallpaperSource;
    m_settings->save("Wallpaper", m_wallpaperSource);
    emit wallpaperSourceChanged(m_wallpaperSource);
}

void Background::setDimWallpaper(bool dimWallpaper)
{
    if (m_dimWallpaper == dimWallpaper)
        return;

    m_dimWallpaper = dimWallpaper;
    m_settings->save("DimWallpaper", m_dimWallpaper);
    emit dimWallpaperChanged(m_dimWallpaper);
}

void Background::setAdaptiveColorScheme(bool adaptiveColorScheme)
{
    if (m_adaptiveColorScheme == adaptiveColorScheme)
        return;

    m_adaptiveColorScheme = adaptiveColorScheme;
    m_settings->save("AdaptiveColorScheme", m_adaptiveColorScheme);
    emit adaptiveColorSchemeChanged(m_adaptiveColorScheme);
}

void Background::setFitWallpaper(bool fitWallpaper)
{
    if (m_fitWallpaper == fitWallpaper)
        return;

    m_fitWallpaper = fitWallpaper;
    m_settings->save("FitWallpaper", m_fitWallpaper);
    emit fitWallpaperChanged(m_fitWallpaper);
}

void Background::setSolidColor(QString solidColor)
{
    if (m_solidColor == solidColor)
        return;

    m_solidColor = solidColor;
    m_settings->save("SolidColor", m_solidColor);
    emit solidColorChanged(m_solidColor);
}

void Background::setShowWallpaper(bool showWallpaper)
{
    if (m_showWallpaper == showWallpaper)
        return;

    m_showWallpaper = showWallpaper;
    m_settings->save("ShowWallpaper", m_showWallpaper);
    emit showWallpaperChanged(m_showWallpaper);
}


