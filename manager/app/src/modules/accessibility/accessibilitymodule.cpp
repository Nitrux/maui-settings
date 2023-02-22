#include "accessibilitymodule.h"
#include <KI18n/KLocalizedString>

#include <MauiMan/accessibilitymanager.h>

AccessibilityModule::AccessibilityModule(QObject *parent) : AbstractModule("accessibility",
                                                                           i18n("Accessibility"),
                                                                            i18n("System"),
                                                                           "qrc:/modules/accessibility/AccessibilityModuleView.qml",
                                                                           "preferences-desktop-accessibility",
                                                                            i18n("Customize accessibility features."),
                                                                           {i18n("accessibility"), i18n("click")},
                                                                           parent)
  ,m_manager(nullptr)
{
    qRegisterMetaType<MauiMan::AccessibilityManager*>("const MauiMan::AccessibilityManager*"); // this is needed for QML to know of AccessibilityManager

}

MauiMan::AccessibilityManager *AccessibilityModule::manager()
{
    if(!m_manager)
    {
        m_manager = new MauiMan::AccessibilityManager(this);
    }

    return m_manager;
}

