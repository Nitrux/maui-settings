#include "accountsmodule.h"


AccountsModule::AccountsModule(QObject *parent) :AbstractModule(QStringLiteral("accounts"), parent)
  , m_manager(nullptr)
{
}


QString AccountsModule::qmlSource() const
{
    return ("qrc:/modules/theme/AccountsModuleView.qml");
}

QString AccountsModule::iconName() const
{
    return "theme-config";
}

QString AccountsModule::name() const
{
    return "Theme";
}

QString AccountsModule::category() const
{
    return "Apperance";
}

QStringList AccountsModule::keywords() const
{
    return {"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"};
}


QString AccountsModule::description() const
{
    return "Accent colors, icon sets, adaptive colorscheme.";
}

