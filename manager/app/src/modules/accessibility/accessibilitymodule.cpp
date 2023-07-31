#include "accessibilitymodule.h"
#include <KLocalizedString>


AccessibilityModule::AccessibilityModule(QObject *parent) : AbstractModule(QStringLiteral("accessibility"),
                                                                           i18n("Accessibility"),
                                                                           i18n("System"),
                                                                           QStringLiteral("qrc:/MauiSettings/src/modules/accessibility/AccessibilityModuleView.qml"),
                                                                           QStringLiteral("preferences-desktop-accessibility"),
                                                                           i18n("Customize accessibility features."),

                                                                           QStringList{i18n("accessibility"),
                                                                           i18n("click")},

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

