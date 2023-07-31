#include "formfactormodule.h"

#include <KLocalizedString>

FormFactorModule::FormFactorModule(QObject *parent) : AbstractModule(QStringLiteral("formfactor"),
                                                                     i18n("Form Factor"),
                                                                     i18n("System"),
                                                                     QStringLiteral("qrc:/MauiSettings/src/modules/formfactor/FormFactorModuleView.qml"),
                                                                     QStringLiteral("preferences-desktop-wallpaper"),
                                                                     i18n("Switch between desktop and mobile modes."),

                                                                     QStringList{i18n("form factor"),
                                                                     i18n("phone"),
                                                                     i18n("tablet"),
                                                                     i18n("mobile"),
                                                                     i18n("input")},

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
