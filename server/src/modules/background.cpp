#include "background.h"
#include "backgroundadaptor.h"
#include <QDBusInterface>

#include "code/settingsstore.h"

#include <QDebug>

Background::Background(QObject *parent) : QObject(parent)
{
    qDebug( " INIT BACKGORUND MANAGER");
    new BackgroundAdaptor(this);
    if(!QDBusConnection::sessionBus().registerObject(QStringLiteral("/Background"), this))
    {
        qDebug() << "FAILED TO REGISTER BACKGROUND DBUS OBJECT";
        return;
    }
    MauiMan::SettingsStore settings;
    settings.beginModule("Background");
    m_wallpaperSource = settings.load("Wallpaper", m_wallpaperSource).toString();
    m_dimWallpaper = settings.load("DimWallpaper", m_dimWallpaper).toBool();
    m_fitWallpaper = settings.load("FitWallpaper", m_fitWallpaper).toBool();
    m_showWallpaper = settings.load("ShowWallpaper", m_showWallpaper).toBool();
    m_solidColor = settings.load("SolidColor", m_solidColor).toString();
    settings.endModule();
}

QString Background::wallpaperSource() const
{
    return m_wallpaperSource;
}

bool Background::dimWallpaper() const
{
    return m_dimWallpaper;
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
    emit wallpaperSourceChanged(m_wallpaperSource);
}

void Background::setDimWallpaper(bool dimWallpaper)
{
    if (m_dimWallpaper == dimWallpaper)
        return;

    m_dimWallpaper = dimWallpaper;
    emit dimWallpaperChanged(m_dimWallpaper);
}

void Background::setFitWallpaper(bool fitWallpaper)
{
    if (m_fitWallpaper == fitWallpaper)
        return;

    m_fitWallpaper = fitWallpaper;
    emit fitWallpaperChanged(m_fitWallpaper);
}

void Background::setSolidColor(QString solidColor)
{
    if (m_solidColor == solidColor)
        return;

    m_solidColor = solidColor;
    emit solidColorChanged(m_solidColor);
}

void Background::setShowWallpaper(bool showWallpaper)
{
    if (m_showWallpaper == showWallpaper)
        return;

    m_showWallpaper = showWallpaper;
    emit showWallpaperChanged(m_showWallpaper);
}


