#include "aboutmodule.h"
#include "aboutdata.h"

AboutModule::AboutModule(QObject *parent) :AbstractModule("about", parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<AboutData *>("const AboutData*"); // this is needed for QML to know of FMList in the search method

}


QString AboutModule::qmlSource() const
{
    return ("qrc:/modules/about/AboutModuleView.qml");
}

QString AboutModule::iconName() const
{
    return "user-man";
}

QString AboutModule::name() const
{
    return "About";
}

QString AboutModule::category() const
{
    return "General";
}

QStringList AboutModule::keywords() const
{
    return {"about", "info", "information", "user", "data"};
}


QString AboutModule::description() const
{
    return "User and system information.";
}

AboutData *AboutModule::manager()
{
    if(!m_manager)
    {
        m_manager = new AboutData(this);
    }
    return m_manager;
}
