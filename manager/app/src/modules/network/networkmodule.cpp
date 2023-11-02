#include "networkmodule.h"
//#include "theme.h"
#include <KLocalizedString>


NetworkModule::NetworkModule(QObject *parent) :AbstractModule(QStringLiteral("network"),
                                                              i18n("Network"),
                                                              i18n("Network"),
                                                              QStringLiteral("qrc:/MauiSettings/src/modules/network/NetworkModuleView.qml"),
                                                              QStringLiteral("preferences-system-network"),
                                                              i18n("Accent colors, icon sets, adaptive colorscheme."),

                                                              QStringList{i18n("look"),
                                                              i18n("theme"),
                                                              i18n("color"),
                                                              i18n("icons"),
                                                              i18n("dark mode"),
                                                              i18n("dark"),
                                                              i18n("adaptive")},

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
