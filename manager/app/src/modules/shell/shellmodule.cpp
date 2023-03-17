#include "shellmodule.h"
//#include "theme.h"
#include <KI18n/KLocalizedString>


ShellModule::ShellModule(QObject *parent) :AbstractModule(QStringLiteral("shell"),
                                                          i18n("Shell"),
                                                          i18n("System"),
                                                          QStringLiteral("qrc:/modules/theme/ShellModuleView.qml"),
                                                          QStringLiteral("preferences-desktop"),
                                                          i18n("Accent colors, icon sets, adaptive colorscheme."),
                                                          QStringList{"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"},                                                          parent)
//  , m_manager(nullptr)
{
//    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of FMList in the search method

}


//Theme *ShellModule::manager()
//{
//    if(!m_manager)
//    {
//        m_manager = new Theme(this);
//    }

//    return m_manager;
//}
