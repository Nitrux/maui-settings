#include "backgroundmodule.h"
#include "background.h"

#include <KLocalizedString>


BackgroundModule::BackgroundModule(QObject *parent) :AbstractModule(QStringLiteral("background"),
                                                                    i18n("Background"),
                                                                    i18n("Apperance"),
                                                                    QStringLiteral("qrc:/MauiSettings/src/modules/background/BackgroundModuleView.qml"),
                                                                    QStringLiteral("preferences-desktop-wallpaper"),
                                                                    i18n("Wallpaper color, sources, aspect and more"),

                                                                    QStringList{i18n("look"),
                                                                    i18n("wallpaper"),
                                                                    i18n("apperance"),
                                                                    i18n("background")},

                                                                    parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<Background*>("const Background*"); // this is needed for QML to know of FMList in the search method
}

Background *BackgroundModule::manager()
{
    if(!m_manager)
    {
        m_manager = new Background(this);
    }

    return m_manager;
}
