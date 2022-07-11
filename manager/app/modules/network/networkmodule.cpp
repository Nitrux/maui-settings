#include "networkmodule.h"
//#include "theme.h"


NetworkModule::NetworkModule(QObject *parent) :AbstractModule(parent)
//  , m_manager(nullptr)
{
//    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of FMList in the search method

}


QString NetworkModule::qmlSource() const
{
    return ("qrc:/modules/theme/NetworkModuleView.qml");
}

QString NetworkModule::iconName() const
{
    return "theme-config";
}

QString NetworkModule::name() const
{
    return "Network";
}

QString NetworkModule::category() const
{
    return "Network";
}

QStringList NetworkModule::keywords() const
{
    return {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"};
}


QString NetworkModule::description() const
{
    return "Accent colors, icon sets, adaptive colorscheme.";
}

//Theme *NetworkModule::manager()
//{
//    if(!m_manager)
//    {
//        m_manager = new Theme(this);
//    }

//    return m_manager;
//}
