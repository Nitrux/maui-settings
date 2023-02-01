#include "accessibilitymodule.h"
#include <KI18n/KLocalizedString>

#include <MauiMan/accessibilitymanager.h>

AccessibilityModule::AccessibilityModule(QObject *parent) : AbstractModule("accessibility", parent)
  ,m_manager(nullptr)
{
    qRegisterMetaType<MauiMan::AccessibilityManager*>("const MauiMan::AccessibilityManager*"); // this is needed for QML to know of AccessibilityManager

}


QString AccessibilityModule::qmlSource() const
{
    return ("qrc:/modules/accessibility/AccessibilityModuleView.qml");
}

QString AccessibilityModule::iconName() const
{
    return "preferences-desktop-accessibility";
}

QString AccessibilityModule::name() const
{
    return i18n("Accessibility");
}

QString AccessibilityModule::category() const
{
    return i18n("System");
}

QStringList AccessibilityModule::keywords() const
{
    return{i18n("accessibility"), i18n("click")};
}

QString AccessibilityModule::description() const
{
    return i18n("Customize accessibility features.");
}

MauiMan::AccessibilityManager *AccessibilityModule::manager()
{
    if(!m_manager)
    {
        m_manager = new MauiMan::AccessibilityManager(this);
    }

    return m_manager;
}

