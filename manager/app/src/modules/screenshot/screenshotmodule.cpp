#include "screenshotmodule.h"
//#include "theme.h"
#include <KI18n/KLocalizedString>

#include "screenshothandler.h"

ScreenshotModule::ScreenshotModule(QObject *parent) :AbstractModule("screenshot",
                                                          i18n("Screenshot"),
                                                          i18n("Shell"),
                                                          "qrc:/modules/screenshot/ScreenshotModuleView.qml",
                                                          "preferences-desktop",
                                                          i18n("Accent colors, icon sets, adaptive colorscheme."),
                                                          {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"},                                                          parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<ScreenshotHandler *>("const ScreenshotHandler*"); // this is needed for QML to know of Theme

}


ScreenshotHandler *ScreenshotModule::manager()
{
    if(!m_manager)
    {
        m_manager = new ScreenshotHandler(this);
    }

    return m_manager;
}
