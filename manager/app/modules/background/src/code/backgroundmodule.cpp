#include "backgroundmodule.h"
#include "background.h"


BackgroundModule::BackgroundModule(QObject *parent) :AbstractModule(parent)
  , m_manager(new Background(this))
{
    qRegisterMetaType<Background*>("const Background*"); // this is needed for QML to know of FMList in the search method

}

QString BackgroundModule::qmlSource() const
{
    return ("qrc:/modules/background/controls/BackgroundModuleView.qml");
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

Background *BackgroundModule::manager() const
{
    return m_manager;
}
