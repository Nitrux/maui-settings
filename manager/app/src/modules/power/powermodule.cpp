#include "thememodule.h"
#include "theme.h"


ThemeModule::ThemeModule(QObject *parent) :AbstractModule(parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of FMList in the search method

}


QString ThemeModule::qmlSource() const
{
    return ("qrc:/modules/theme/ThemeModuleView.qml");
}

QString ThemeModule::iconName() const
{
    return "theme-config";
}

QString ThemeModule::name() const
{
    return "Theme";
}

QString ThemeModule::category() const
{
    return "Apperance";
}

QStringList ThemeModule::keywords() const
{
    return {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"};
}


QString ThemeModule::description() const
{
    return "Accent colors, icon sets, adaptive colorscheme.";
}

Theme *ThemeModule::manager()
{
    if(!m_manager)
    {
        m_manager = new Theme(this);
    }

    return m_manager;
}
