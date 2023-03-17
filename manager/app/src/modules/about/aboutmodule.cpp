#include "aboutmodule.h"
#include "aboutdata.h"

#include <KI18n/KLocalizedString>

AboutModule::AboutModule(QObject *parent) :AbstractModule(QStringLiteral("about"),
                                                          i18n("About"),
                                                          i18n("General"),
                                                          QStringLiteral("qrc:/modules/about/AboutModuleView.qml"),
                                                          QStringLiteral("user-man"),
                                                          QStringLiteral("User and system information."),
                                                          QStringList{"about", "info", "information", "user", "data"},
                                                          parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<AboutData *>("const AboutData*"); // this is needed for QML to know of FMList in the search method

}

AboutData *AboutModule::manager()
{
    if(!m_manager)
    {
        m_manager = new AboutData(this);
    }
    return m_manager;
}
