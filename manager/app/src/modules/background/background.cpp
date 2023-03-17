#include "background.h"

#include <MauiMan/settingsstore.h>

#include <QDebug>

Background::Background(QObject *parent) : MauiMan::BackgroundManager(parent)
,m_wallpaperSourceDir(m_defaultWallpaperSourceDir)
{
    MauiMan::SettingsStore setting;
    setting.beginModule(QStringLiteral("Background"));
    m_wallpaperSourceDir = setting.load(QStringLiteral("SourceDir"), m_wallpaperSourceDir).toString();
    setting.endModule();
}

QString Background::wallpaperSourceDir() const
{
    return m_wallpaperSourceDir;
}

void Background::setWallpaperSourceDir(QString wallpaperSourceDir)
{
    if (m_wallpaperSourceDir == wallpaperSourceDir)
        return;

    m_wallpaperSourceDir = wallpaperSourceDir;
    MauiMan::SettingsStore setting;
    setting.beginModule(QStringLiteral("Background"));
    setting.save(QStringLiteral("SourceDir"), m_wallpaperSourceDir);
    setting.endModule();
    emit wallpaperSourceDirChanged(m_wallpaperSourceDir);
}

void Background::resetWallpaperSourceDir()
{
    setWallpaperSourceDir(m_defaultWallpaperSourceDir);
}
