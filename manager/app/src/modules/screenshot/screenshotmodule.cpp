#include "screenshotmodule.h"
//#include "theme.h"
#include <KLocalizedString>

#include "screenshothandler.h"

ScreenshotModule::ScreenshotModule(QObject *parent) :AbstractModule(QStringLiteral("screenshot"),
                                                                    i18n("Screenshot"),
                                                                    i18n("Shell"),
                                                                    QStringLiteral("qrc:/MauiSettings/src/modules/screenshot/ScreenshotModuleView.qml"),
                                                                    QStringLiteral("preferences-desktop"),
                                                                    i18n("Accent colors, icon sets, adaptive colorscheme."),

                                                                    QStringList{i18n("look"),
                                                                    i18n("theme"),
                                                                    i18n("color"),
                                                                    i18n("icons"),
                                                                    i18n("dark mode"),
                                                                    i18n("dark"),
                                                                    i18n("adaptive")},

                                                                    parent)
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
