#include "backgroundmodule.h"

BackgroundModule::BackgroundModule(QObject *parent) :AbstractModule(parent)
{

}


QUrl BackgroundModule::qmlSource() const
{
    return QUrl();
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
