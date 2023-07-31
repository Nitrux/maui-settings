#include "screenmodule.h"
#include "screen.h"
#include <KLocalizedString>

ScreenModule::ScreenModule(QObject *parent) :AbstractModule(QStringLiteral("screen"),
                                                            i18n("Screen"),
                                                            i18n("System"),
                                                            QStringLiteral("qrc:/MauiSettings/src/modules/screen/ScreenModuleView.qml"),
                                                            QStringLiteral("cs-screen"),
                                                            i18n("Screen properties."),

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
    qRegisterMetaType<Screen *>("const Screen*"); // this is needed for QML to know of FMList in the search method

}

Screen *ScreenModule::manager()
{
    if(!m_manager)
    {
        m_manager = new Screen(this);
    }

    return m_manager;
}
