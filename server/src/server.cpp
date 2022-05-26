#include "server.h"
#include <QDBusConnectionInterface>

#include "src/modules/background.h"
#include "src/modules/theme.h"

Server::Server(int &argc, char **argv) : QCoreApplication(argc, argv)
{
    QDBusConnectionInterface *iface = QDBusConnection::sessionBus().interface();

    auto registration = iface->registerService(QStringLiteral("org.mauiman.Manager"),
                                               QDBusConnectionInterface::ReplaceExistingService,
                                               QDBusConnectionInterface::DontAllowReplacement);

    if (!registration.isValid())
    {
        qWarning("2 Failed to register D-Bus service \"%s\" on session bus: \"%s\"",
                 qPrintable("org.mauiman.Manager"),
                 qPrintable(registration.error().message()));
    }

    new Background(this);
    new Theme(this);
}
