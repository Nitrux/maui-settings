#include "backgroundmodule.h"
#include "background.h"
#include <KI18n/KLocalizedString>


BackgroundModule::BackgroundModule(QObject *parent) :AbstractModule("background",
                                                                    i18n("Background"),
                                                                    i18n("Apperance"),
                                                                    "qrc:/modules/background/BackgroundModuleView.qml",
                                                                    "preferences-desktop-wallpaper",
                                                                    i18n("Wallpaper color, sources, aspect and more"),
                                                                     {"look", "wallpaper", "apperance", "background"},
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
