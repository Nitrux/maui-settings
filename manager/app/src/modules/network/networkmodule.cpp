#include "networkmodule.h"
//#include "theme.h"
#include <KI18n/KLocalizedString>


NetworkModule::NetworkModule(QObject *parent) :AbstractModule("network",
                                                              i18n("Network"),
                                                              i18n("Network"),
                                                              "qrc:/modules/network/NetworkModuleView.qml",
                                                              "preferences-system-network",
                                                              i18n("Accent colors, icon sets, adaptive colorscheme."),
                                                              {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"},
                                                              parent)
//  , m_manager(nullptr)
{
//    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of FMList in the search method

}

//Theme *NetworkModule::manager()
//{
//    if(!m_manager)
//    {
//        m_manager = new Theme(this);
//    }

//    return m_manager;
//}
