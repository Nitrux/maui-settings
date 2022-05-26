#include "mauimanutils.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusServiceWatcher>

#include <QDebug>

static const QString mauimanInterface(QStringLiteral("org.mauiman.Manager"));

MauiManUtils::MauiManUtils(QObject *parent)
    : QObject{parent}
{
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
}

bool MauiManUtils::serverRunning() const
{
    return m_serverRunning;
}

void MauiManUtils::startServer()
{

}

void MauiManUtils::invokeManager(const QString &module)
{

}
