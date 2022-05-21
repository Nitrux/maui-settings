#include "backgroundman.h"

//#include "code/settingsstore.h"

#include <QDebug>

//using namespace MauiMan;
BackgroundMan::BackgroundMan(QObject *parent) : QObject(parent)
//  ,m_backgroundInterface("org.mauiman.Manager",
//                         "/Background",
//                         "org.mauiman.Background",
//                         QDBusConnection::sessionBus(), this)
{
    qDebug( " INIT BACKGORUND MANAGER");
//    SettingsStore settings;
//    settings.beginModule("Background");
//    m_wallpaperSource = settings.load("Wallpaper", m_wallpaperSource).toString();
//    m_dimWallpaper = settings.load("DimWallpaper", m_dimWallpaper).toBool();
//    m_showWallpaper = settings.load("ShowWallpaper", m_showWallpaper).toBool();
//    m_adaptiveColorScheme = settings.load("AdaptiveColorScheme", m_adaptiveColorScheme).toBool();
//    m_fitWallpaper = settings.load("FitWallpaper", m_fitWallpaper).toBool();
//    m_solidColor = settings.load("SolidColor", m_solidColor).toString();
//    settings.endModule();

//    if (m_backgroundInterface.isValid())
//    {
//        connect(&m_backgroundInterface, SIGNAL(wallpaperSourceChanged(QString)), this, SLOT(onWallpaperChanged(QString)));
//        connect(&m_backgroundInterface, SIGNAL(solidColorChanged(QString)), this, SLOT(onSolidColorChanged(QString)));

//    }
}
