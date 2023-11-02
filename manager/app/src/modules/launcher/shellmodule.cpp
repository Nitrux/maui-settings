#include "shellmodule.h"
//#include "theme.h"
#include <KLocalizedString>


ShellModule::ShellModule(QObject *parent) :AbstractModule(QStringLiteral("shell"),
                                                          i18n("Shell"),
                                                          i18n("System"),
                                                          QStringLiteral("qrc:/modules/theme/ShellModuleView.qml"),
                                                          QStringLiteral("preferences-desktop"),
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


//Theme *ShellModule::manager()
//{
//    if(!m_manager)
//    {
//        m_manager = new Theme(this);
//    }

//    return m_manager;
//}
