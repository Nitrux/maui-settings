#include "modulesmanager.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusServiceWatcher>

#include <QDir>
#include <QDebug>
#include <QQmlEngine>

//modules
#include "modules/background/src/code/backgroundmodule.h"
#include "modules/theme/code/thememodule.h"

static const QString mauimanInterface(QStringLiteral("org.mauiman.Manager"));


ModulesManager::ModulesManager(QObject *parent) : QObject(parent)
  ,m_model(new ModulesModel(this))
{
//    QString path = MAUIMAN_MODULES_IMPORT_PATH;
//    QQmlEngine engine;
    const QDBusConnection bus = QDBusConnection::sessionBus();
    const auto registeredServices = bus.interface()->registeredServiceNames();

    if (registeredServices.isValid())
    {
        m_serverRunning = registeredServices.value().contains(mauimanInterface);
    }

    auto watcher = new QDBusServiceWatcher(mauimanInterface, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration, this);

    connect(watcher, &QDBusServiceWatcher::serviceRegistered, [=](const QString &name) {
        qDebug() << "Found MauiMan server" << name;
        m_serverRunning = true;
emit serverRunningChanged(m_serverRunning);
    });

    connect(watcher, &QDBusServiceWatcher::serviceUnregistered, [=](const QString &name) {
        qDebug() << "Found MauiMan server" << name;
        m_serverRunning = false;
        emit serverRunningChanged(m_serverRunning);


    });

    m_model->appendModule(new BackgroundModule);
    m_model->appendModule(new ThemeModule);
}

ModulesModel *ModulesManager::model() const
{
    return m_model;
}

bool ModulesManager::serverRunning() const
{
    return m_serverRunning;
}
