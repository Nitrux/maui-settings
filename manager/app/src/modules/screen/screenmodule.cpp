#include "screenmodule.h"
#include "screen.h"
#include <KI18n/KLocalizedString>

ScreenModule::ScreenModule(QObject *parent) :AbstractModule("screen",
                                                            i18n("Screen"),
                                                            i18n("System"),
                                                            "qrc:/modules/screen/ScreenModuleView.qml",
                                                            "cs-screen",
                                                            i18n("Screen properties."),
                                                            {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"},
                                                            parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<Screen *>("const Screen*"); // this is needed for QML to know of FMList in the search method

}

Screen *ScreenModule::manager()
{
    if(!m_manager)
    {
        m_manager = new Screen(this);
    }

    return m_manager;
}
