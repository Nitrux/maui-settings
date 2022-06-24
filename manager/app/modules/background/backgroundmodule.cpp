#include "backgroundmodule.h"
#include "background.h"


BackgroundModule::BackgroundModule(QObject *parent) :AbstractModule(parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<Background*>("const Background*"); // this is needed for QML to know of FMList in the search method

}

QString BackgroundModule::qmlSource() const
{
    return ("qrc:/modules/background/BackgroundModuleView.qml");
}

QString BackgroundModule::iconName() const
{
    return "preferences-desktop-wallpaper";
}

QString BackgroundModule::name() const
{
    return "Background";
}

QString BackgroundModule::category() const
{
    return "Apperance";
}

QStringList BackgroundModule::keywords() const
{
    return {"look", "wallpaper", "apperance", "background"};
}


QString BackgroundModule::description() const
{
    return "Wallpaper color, sources, aspect and more";
}

Background *BackgroundModule::manager()
{
    if(!m_manager)
    {
        m_manager = new Background(this);
    }

    return m_manager;
}
