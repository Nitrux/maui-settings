#include "background.h"

#include <MauiMan/settingsstore.h>

#include <QDebug>

Background::Background(QObject *parent) : MauiMan::BackgroundManager(parent)
{
    MauiMan::SettingsStore setting;
    setting.beginModule("Background");
    m_wallpaperSourceDir = setting.load("SourceDir", m_wallpaperSourceDir).toString();
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
    setting.beginModule("Background");
    setting.save("SourceDir", m_wallpaperSourceDir);
    setting.endModule();
    emit wallpaperSourceDirChanged(m_wallpaperSourceDir);
}
