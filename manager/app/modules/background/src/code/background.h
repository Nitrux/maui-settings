#pragma once

#include <QObject>
#include <QString>

#include "code/modules/backgroundmanager.h"

class Background : public MauiMan::BackgroundManager
{
    Q_OBJECT
    Q_PROPERTY(QString wallpaperSourceDir READ wallpaperSourceDir WRITE setWallpaperSourceDir NOTIFY wallpaperSourceDirChanged)

public:
    explicit Background(QObject * parent = nullptr);

    QString wallpaperSourceDir() const;
    void setWallpaperSourceDir(QString wallpaperSourceDir);

signals:

    void wallpaperSourceDirChanged(QString wallpaperSourceDir);

private:

    QString m_wallpaperSourceDir = "file:///usr/share/wallpapers/Cask";

};

