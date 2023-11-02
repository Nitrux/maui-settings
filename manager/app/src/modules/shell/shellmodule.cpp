#include "shellmodule.h"
//#include "theme.h"
#include <KLocalizedString>


ShellModule::ShellModule(QObject *parent) :AbstractModule(QStringLiteral("workspace"),
                                                          i18n("Workspace"),
                                                          i18n("Shell"),
                                                          QStringLiteral("qrc:/MauiSettings/src/modules/theme/ShellModuleView.qml"),
                                                          QStringLiteral("preferences-desktop-workspaces"),
                                                          i18n("Accent colors, icon sets, adaptive colorscheme."),

                                                          QStringList{i18n("dock"),
                                                          i18n("panel"),
                                                          QStringLiteral("cask"),
                                                          i18n("workspace"),
                                                          i18n("launcher")},

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
