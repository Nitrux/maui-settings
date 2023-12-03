#include "aboutmodule.h"

#include <KLocalizedString>

AboutModule::AboutModule(QObject *parent) :AbstractModule(QStringLiteral("about"),
                                                          i18n("About"),
                                                          i18n("General"),
                                                          QStringLiteral("qrc:/MauiSettingsModule/src/modules/about/AboutModuleView.qml"),
                                                          QStringLiteral("user-man"),
                                                          QStringLiteral("User and system information."),

                                                          QStringList{i18n("about"),
                                                          i18n("info"),
                                                          i18n("information"),
                                                          i18n("user"),
                                                          i18n("data")},

                                                          parent)
  , m_manager(nullptr)
{
}

AboutData *AboutModule::manager()
{
    if(!m_manager)
    {
        m_manager = new AboutData(this);
    }
    return m_manager;
}
