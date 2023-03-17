#include "formfactormodule.h"

#include <MauiMan/formfactormanager.h>
#include <KI18n/KLocalizedString>


FormFactorModule::FormFactorModule(QObject *parent) : AbstractModule(QStringLiteral("formfactor"),
                                                                     i18n("Form Factor"),
                                                                     i18n("System"),
                                                                     QStringLiteral("qrc:/modules/formfactor/FormFactorModuleView.qml"),
                                                                     QStringLiteral("preferences-desktop-wallpaper"),
                                                                     i18n("Switch between desktop and mobile modes."),
                                                                     QStringList{"form factor", "phone", "tablet", "mobile", "input"},
                                                                     parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<MauiMan::FormFactorManager*>("const MauiMan::FormFactorManager*"); // this is needed for QML to know of FormFactorManager

}

MauiMan::FormFactorManager *FormFactorModule::manager()
{
    if(!m_manager)
    {
        m_manager = new MauiMan::FormFactorManager(this);
    }

    return m_manager;
}
