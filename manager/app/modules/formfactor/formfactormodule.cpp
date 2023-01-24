#include "formfactormodule.h"

#include <MauiMan/formfactormanager.h>
#include <KI18n/KLocalizedString>


FormFactorModule::FormFactorModule(QObject *parent) :AbstractModule(parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<MauiMan::FormFactorManager*>("const MauiMan::FormFactorManager*"); // this is needed for QML to know of FMList in the search method

}

QString FormFactorModule::qmlSource() const
{
    return ("qrc:/modules/formfactor/FormFactorModuleView.qml");
}

QString FormFactorModule::iconName() const
{
    return "preferences-desktop-wallpaper";
}

QString FormFactorModule::name() const
{
    return i18n("Form Factor");
}

QString FormFactorModule::category() const
{
    return i18n("System");
}

QStringList FormFactorModule::keywords() const
{
    return {"form factor", "phone", "tablet", "mobile", "input"};
}

QString FormFactorModule::description() const
{
    return i18n("Switch between desktop and mobile modes.");
}

MauiMan::FormFactorManager *FormFactorModule::manager()
{
    if(!m_manager)
    {
        m_manager = new MauiMan::FormFactorManager(this);
    }

    return m_manager;
}
