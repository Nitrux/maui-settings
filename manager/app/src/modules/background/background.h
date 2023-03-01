#pragma once

#include <QObject>
#include <QString>

#include <MauiMan/backgroundmanager.h>

class Background : public MauiMan::BackgroundManager
{
    Q_OBJECT
    Q_PROPERTY(QString wallpaperSourceDir READ wallpaperSourceDir WRITE setWallpaperSourceDir NOTIFY wallpaperSourceDirChanged RESET resetWallpaperSourceDir)

public:
    explicit Background(QObject * parent = nullptr);

    QString wallpaperSourceDir() const;
    void setWallpaperSourceDir(QString wallpaperSourceDir);
 void resetWallpaperSourceDir();
signals:
    void wallpaperSourceDirChanged(QString wallpaperSourceDir);

private:

    QString m_defaultWallpaperSourceDir = "file:///usr/share/wallpapers/Cask";
    QString m_wallpaperSourceDir;

};

