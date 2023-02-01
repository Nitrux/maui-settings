#include "shellmodule.h"
//#include "theme.h"


ShellModule::ShellModule(QObject *parent) :AbstractModule("shell", parent)
//  , m_manager(nullptr)
{
//    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of FMList in the search method

}


QString ShellModule::qmlSource() const
{
    return ("qrc:/modules/theme/ShellModuleView.qml");
}

QString ShellModule::iconName() const
{
    return "preferences-desktop";
}

QString ShellModule::name() const
{
    return "Shell";
}

QString ShellModule::category() const
{
    return "System";
}

QStringList ShellModule::keywords() const
{
    return {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"};
}


QString ShellModule::description() const
{
    return "Accent colors, icon sets, adaptive colorscheme.";
}

//Theme *ShellModule::manager()
//{
//    if(!m_manager)
//    {
//        m_manager = new Theme(this);
//    }

//    return m_manager;
//}
