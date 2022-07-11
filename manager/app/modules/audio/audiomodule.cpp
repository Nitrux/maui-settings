#include "audiomodule.h"
//#include "theme.h"


AudioModule::AudioModule(QObject *parent) :AbstractModule(parent)
//  , m_manager(nullptr)
{
//    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of FMList in the search method

}


QString AudioModule::qmlSource() const
{
    return ("qrc:/modules/theme/AudioModuleView.qml");
}

QString AudioModule::iconName() const
{
    return "theme-config";
}

QString AudioModule::name() const
{
    return "Audio";
}

QString AudioModule::category() const
{
    return "System";
}

QStringList AudioModule::keywords() const
{
    return {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"};
}


QString AudioModule::description() const
{
    return "Accent colors, icon sets, adaptive colorscheme.";
}

//Theme *AudioModule::manager()
//{
//    if(!m_manager)
//    {
//        m_manager = new Theme(this);
//    }

//    return m_manager;
//}
