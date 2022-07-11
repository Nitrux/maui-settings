#include "screenmodule.h"
#include "screen.h"

ScreenModule::ScreenModule(QObject *parent) :AbstractModule(parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<Screen *>("const Screen*"); // this is needed for QML to know of FMList in the search method

}


QString ScreenModule::qmlSource() const
{
    return ("qrc:/modules/screen/ScreenModuleView.qml");
}

QString ScreenModule::iconName() const
{
    return "cs-screen";
}

QString ScreenModule::name() const
{
    return "Screen";
}

QString ScreenModule::category() const
{
    return "System";
}

QStringList ScreenModule::keywords() const
{
    return {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"};
}


QString ScreenModule::description() const
{
    return "Screen properties.";
}

Screen *ScreenModule::manager()
{
    if(!m_manager)
    {
        m_manager = new Screen(this);
    }

    return m_manager;
}
