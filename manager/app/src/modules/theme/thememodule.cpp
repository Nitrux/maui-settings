#include "thememodule.h"
#include "theme.h"

#include <KI18n/KLocalizedString>


ThemeModule::ThemeModule(QObject *parent) :AbstractModule("theme",
                                                          i18n("Theme"),
                                                          i18n("Apperance"),
                                                          "qrc:/modules/theme/ThemeModuleView.qml",
                                                          "preferences-desktop-theme",
                                                          i18n("Accent colors, icon sets, adaptive colorscheme."),
                                                          {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"},
                                                          parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of Theme
}

Theme *ThemeModule::manager()
{
    if(!m_manager)
    {
        m_manager = new Theme(this);
    }

    return m_manager;
}
