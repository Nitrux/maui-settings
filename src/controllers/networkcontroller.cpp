#include "networkcontroller.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QTimer>

#include <algorithm>

#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessNetwork>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/WirelessSetting>

namespace
{
struct NetworkEntry {
    QString ssid;
    int signalStrength = 0;
    QString security;
    bool secure = false;
    bool passwordRequired = false;
    bool connected = false;
    bool saved = false;
    bool autoConnect = false;
    QString devicePath;
    QString accessPointPath;
};

QString securityName(NetworkManager::WirelessSecurityType type)
{
    switch (type) {
    case NetworkManager::NoneSecurity:
        return QStringLiteral("Open");
    case NetworkManager::StaticWep:
        return QStringLiteral("WEP");
    case NetworkManager::WpaPsk:
    case NetworkManager::Wpa2Psk:
        return QStringLiteral("WPA/WPA2 Personal");
    case NetworkManager::SAE:
        return QStringLiteral("WPA3 Personal");
    case NetworkManager::OWE:
        return QStringLiteral("Enhanced Open");
    case NetworkManager::WpaEap:
    case NetworkManager::Wpa2Eap:
    case NetworkManager::Wpa3SuiteB192:
    case NetworkManager::DynamicWep:
    case NetworkManager::Leap:
        return QStringLiteral("Enterprise");
    default:
        return QStringLiteral("Secured");
    }
}

NetworkManager::Connection::Ptr savedWirelessConnection(const NetworkManager::WirelessDevice::Ptr &device,
                                                         const QString &ssid)
{
    if (!device) {
        return {};
    }

    for (const auto &connection : device->availableConnections()) {
        const auto settings = connection->settings();
        if (!settings || settings->connectionType() != NetworkManager::ConnectionSettings::Wireless) {
            continue;
        }

        const auto wirelessSetting =
            settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
        if (wirelessSetting && QString::fromUtf8(wirelessSetting->ssid()) == ssid) {
            return connection;
        }
    }
    return {};
}
}

class NetworkController::Private
{
public:
    QList<NetworkEntry> entries;
    bool available = false;
    bool scanning = false;
    int scanRequests = 0;
    QString errorMessage;
    QVariantList wiredConnections;
    QTimer *rebuildTimer = nullptr;
};

NetworkController::NetworkController(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
    d->rebuildTimer = new QTimer(this);
    d->rebuildTimer->setSingleShot(true);
    d->rebuildTimer->setInterval(250);
    connect(d->rebuildTimer, &QTimer::timeout, this, &NetworkController::rebuild);

    auto *notifier = NetworkManager::notifier();
    connect(notifier, &NetworkManager::Notifier::deviceAdded, this, [this] {
        scheduleRebuild();
    });
    connect(notifier, &NetworkManager::Notifier::deviceRemoved, this, [this] {
        scheduleRebuild();
    });
    connect(notifier, &NetworkManager::Notifier::activeConnectionsChanged, this, [this] {
        scheduleRebuild();
    });
    connect(notifier, &NetworkManager::Notifier::wirelessEnabledChanged, this, [this] {
        Q_EMIT wirelessEnabledChanged();
        scheduleRebuild();
    });
    connect(notifier, &NetworkManager::Notifier::wirelessHardwareEnabledChanged, this, [this] {
        Q_EMIT hardwareEnabledChanged();
    });

    rebuild();
}

NetworkController::~NetworkController()
{
    delete d;
}

int NetworkController::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->entries.size();
}

QVariant NetworkController::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= d->entries.size()) {
        return {};
    }

    const auto &entry = d->entries.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case SsidRole:
        return entry.ssid;
    case SignalStrengthRole:
        return entry.signalStrength;
    case SecurityRole:
        return entry.security;
    case SecureRole:
        return entry.secure;
    case PasswordRequiredRole:
        return entry.passwordRequired;
    case ConnectedRole:
        return entry.connected;
    case SavedRole:
        return entry.saved;
    case AutoConnectRole:
        return entry.autoConnect;
    case DevicePathRole:
        return entry.devicePath;
    case AccessPointPathRole:
        return entry.accessPointPath;
    default:
        return {};
    }
}

QHash<int, QByteArray> NetworkController::roleNames() const
{
    return {
        {SsidRole, "ssid"},
        {SignalStrengthRole, "signalStrength"},
        {SecurityRole, "security"},
        {SecureRole, "secure"},
        {PasswordRequiredRole, "passwordRequired"},
        {ConnectedRole, "connected"},
        {SavedRole, "saved"},
        {AutoConnectRole, "autoConnect"},
        {DevicePathRole, "devicePath"},
        {AccessPointPathRole, "accessPointPath"},
    };
}

bool NetworkController::available() const
{
    return d->available;
}

bool NetworkController::wirelessEnabled() const
{
    return NetworkManager::isWirelessEnabled();
}

void NetworkController::setWirelessEnabled(bool enabled)
{
    if (enabled == wirelessEnabled()) {
        return;
    }
    NetworkManager::setWirelessEnabled(enabled);
}

bool NetworkController::hardwareEnabled() const
{
    return NetworkManager::isWirelessHardwareEnabled();
}

bool NetworkController::scanning() const
{
    return d->scanning;
}

QString NetworkController::errorMessage() const
{
    return d->errorMessage;
}

QVariantList NetworkController::wiredConnections() const
{
    return d->wiredConnections;
}

void NetworkController::requestScan()
{
    clearError();
    if (!wirelessEnabled()) {
        setErrorMessage(tr("Turn on Wi-Fi before scanning."));
        return;
    }

    int requests = 0;
    for (const auto &device : NetworkManager::networkInterfaces()) {
        if (device->type() != NetworkManager::Device::Wifi) {
            continue;
        }

        const auto wireless = device.objectCast<NetworkManager::WirelessDevice>();
        if (!wireless || wireless->state() == NetworkManager::Device::Unavailable) {
            continue;
        }

        ++requests;
        ++d->scanRequests;
        setScanning(true);
        auto *watcher = new QDBusPendingCallWatcher(wireless->requestScan(), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
            const QDBusPendingReply<> reply = *watcher;
            if (reply.isError()) {
                setErrorMessage(reply.error().message());
            }
            watcher->deleteLater();
            QTimer::singleShot(500, this, [this] {
                rebuild();
                --d->scanRequests;
                setScanning(d->scanRequests > 0);
            });
        });
    }

    if (requests == 0) {
        setErrorMessage(tr("No usable Wi-Fi adapter was found."));
    }
}

void NetworkController::connectToNetwork(const QString &devicePath,
                                         const QString &accessPointPath,
                                         const QString &ssid,
                                         const QString &password)
{
    clearError();
    const auto device = NetworkManager::findNetworkInterface(devicePath).objectCast<NetworkManager::WirelessDevice>();
    if (!device) {
        setErrorMessage(tr("The Wi-Fi adapter is no longer available."));
        return;
    }

    const auto accessPoint = device->findAccessPoint(accessPointPath);
    if (!accessPoint) {
        setErrorMessage(tr("The selected access point is no longer available. Scan again."));
        return;
    }

    for (const auto &connection : device->availableConnections()) {
        const auto settings = connection->settings();
        if (!settings || settings->connectionType() != NetworkManager::ConnectionSettings::Wireless) {
            continue;
        }

        const auto wirelessSetting =
            settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
        if (wirelessSetting && QString::fromUtf8(wirelessSetting->ssid()) == ssid) {
            auto *watcher =
                new QDBusPendingCallWatcher(NetworkManager::activateConnection(connection->path(), devicePath, accessPointPath), this);
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
                const QDBusPendingReply<QDBusObjectPath> reply = *watcher;
                if (reply.isError()) {
                    setErrorMessage(reply.error().message());
                }
                watcher->deleteLater();
                QTimer::singleShot(300, this, &NetworkController::rebuild);
            });
            return;
        }
    }

    auto settings = NetworkManager::ConnectionSettings::Ptr(
        new NetworkManager::ConnectionSettings(NetworkManager::ConnectionSettings::Wireless));
    settings->setId(ssid);
    settings->setUuid(NetworkManager::ConnectionSettings::createNewUuid());
    settings->setAutoconnect(true);

    auto wirelessSetting = settings->setting(NetworkManager::Setting::Wireless).dynamicCast<NetworkManager::WirelessSetting>();
    wirelessSetting->setInitialized(true);
    wirelessSetting->setSsid(ssid.toUtf8());

    const auto securityType = NetworkManager::findBestWirelessSecurity(device->wirelessCapabilities(),
                                                                       true,
                                                                       accessPoint->mode() == NetworkManager::AccessPoint::Adhoc,
                                                                       accessPoint->capabilities(),
                                                                       accessPoint->wpaFlags(),
                                                                       accessPoint->rsnFlags());

    auto securitySetting =
        settings->setting(NetworkManager::Setting::WirelessSecurity).dynamicCast<NetworkManager::WirelessSecuritySetting>();
    if (securityType != NetworkManager::NoneSecurity) {
        securitySetting->setInitialized(true);
        wirelessSetting->setSecurity(QStringLiteral("802-11-wireless-security"));
    }

    switch (securityType) {
    case NetworkManager::NoneSecurity:
        break;
    case NetworkManager::StaticWep:
        securitySetting->setKeyMgmt(NetworkManager::WirelessSecuritySetting::Wep);
        securitySetting->setWepKey0(password);
        break;
    case NetworkManager::OWE:
        securitySetting->setKeyMgmt(NetworkManager::WirelessSecuritySetting::OWE);
        break;
    case NetworkManager::SAE:
        securitySetting->setKeyMgmt(NetworkManager::WirelessSecuritySetting::SAE);
        securitySetting->setPsk(password);
        break;
    case NetworkManager::WpaPsk:
    case NetworkManager::Wpa2Psk:
        securitySetting->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
        securitySetting->setPsk(password);
        break;
    default:
        setErrorMessage(tr("This enterprise network needs a saved NetworkManager profile with its identity and certificate settings."));
        return;
    }

    auto *watcher =
        new QDBusPendingCallWatcher(NetworkManager::addAndActivateConnection(settings->toMap(), devicePath, accessPointPath), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        const QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> reply = *watcher;
        if (reply.isError()) {
            setErrorMessage(reply.error().message());
        }
        watcher->deleteLater();
        QTimer::singleShot(300, this, &NetworkController::rebuild);
    });
}

void NetworkController::disconnectNetwork(const QString &devicePath)
{
    clearError();
    const auto device = NetworkManager::findNetworkInterface(devicePath);
    const auto activeConnection = device ? device->activeConnection() : NetworkManager::ActiveConnection::Ptr();
    if (!activeConnection) {
        return;
    }

    auto *watcher = new QDBusPendingCallWatcher(NetworkManager::deactivateConnection(activeConnection->path()), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        const QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            setErrorMessage(reply.error().message());
        }
        watcher->deleteLater();
        QTimer::singleShot(300, this, &NetworkController::rebuild);
    });
}

void NetworkController::updateSavedNetwork(const QString &devicePath,
                                           const QString &ssid,
                                           bool autoConnect)
{
    clearError();
    const auto device = NetworkManager::findNetworkInterface(devicePath).objectCast<NetworkManager::WirelessDevice>();
    const auto connection = savedWirelessConnection(device, ssid);
    if (!connection || !connection->settings()) {
        setErrorMessage(tr("The saved network profile is no longer available."));
        return;
    }

    connection->settings()->setAutoconnect(autoConnect);
    auto *watcher = new QDBusPendingCallWatcher(connection->update(connection->settings()->toMap()), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        const QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            setErrorMessage(reply.error().message());
        }
        watcher->deleteLater();
        QTimer::singleShot(300, this, &NetworkController::rebuild);
    });
}

void NetworkController::forgetNetwork(const QString &devicePath, const QString &ssid)
{
    clearError();
    const auto device = NetworkManager::findNetworkInterface(devicePath).objectCast<NetworkManager::WirelessDevice>();
    const auto connection = savedWirelessConnection(device, ssid);
    if (!connection) {
        setErrorMessage(tr("The saved network profile is no longer available."));
        return;
    }

    auto *watcher = new QDBusPendingCallWatcher(connection->remove(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
        const QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            setErrorMessage(reply.error().message());
        }
        watcher->deleteLater();
        QTimer::singleShot(300, this, &NetworkController::rebuild);
    });
}

void NetworkController::clearError()
{
    setErrorMessage({});
}

void NetworkController::rebuild()
{
    QList<NetworkEntry> entries;
    QVariantList wiredConnections;
    bool hasWirelessDevice = false;

    for (const auto &baseDevice : NetworkManager::networkInterfaces()) {
        if (baseDevice->type() == NetworkManager::Device::Ethernet) {
            connect(baseDevice.data(), &NetworkManager::Device::availableConnectionAppeared,
                    this, &NetworkController::scheduleRebuild, Qt::UniqueConnection);
            connect(baseDevice.data(), &NetworkManager::Device::availableConnectionDisappeared,
                    this, &NetworkController::scheduleRebuild, Qt::UniqueConnection);
            connect(baseDevice.data(), &NetworkManager::Device::ipV4ConfigChanged,
                    this, &NetworkController::scheduleRebuild, Qt::UniqueConnection);
            connect(baseDevice.data(), &NetworkManager::Device::stateChanged,
                    this, &NetworkController::scheduleRebuild, Qt::UniqueConnection);

            const auto activeConnection = baseDevice->activeConnection();
            const auto activeProfile = activeConnection ? activeConnection->connection() : NetworkManager::Connection::Ptr();
            const bool connecting = baseDevice->state() >= NetworkManager::Device::Preparing
                && baseDevice->state() < NetworkManager::Device::Activated;
            QString ipAddress;
            if (baseDevice->ipV4Config().isValid() && !baseDevice->ipV4Config().addresses().isEmpty()) {
                ipAddress = baseDevice->ipV4Config().addresses().constFirst().ip().toString();
            }

            bool hasWiredProfile = false;
            for (const auto &connection : baseDevice->availableConnections()) {
                const auto settings = connection->settings();
                if (!settings || settings->connectionType() != NetworkManager::ConnectionSettings::Wired) {
                    continue;
                }

                hasWiredProfile = true;
                connect(connection.data(), &NetworkManager::Connection::updated,
                        this, &NetworkController::scheduleRebuild, Qt::UniqueConnection);

                const bool connected = activeProfile && activeProfile->path() == connection->path()
                    && baseDevice->state() == NetworkManager::Device::Activated;
                const bool profileConnecting = activeProfile && activeProfile->path() == connection->path() && connecting;
                wiredConnections.append(QVariantMap{
                    {QStringLiteral("interfaceName"), baseDevice->interfaceName()},
                    {QStringLiteral("connectionName"), connection->name()},
                    {QStringLiteral("connected"), connected},
                    {QStringLiteral("connecting"), profileConnecting},
                    {QStringLiteral("ipAddress"), connected ? ipAddress : QString()},
                    {QStringLiteral("hasProfile"), true},
                });
            }

            if (!hasWiredProfile) {
                wiredConnections.append(QVariantMap{
                    {QStringLiteral("interfaceName"), baseDevice->interfaceName()},
                    {QStringLiteral("connectionName"), QString()},
                    {QStringLiteral("connected"), baseDevice->state() == NetworkManager::Device::Activated},
                    {QStringLiteral("connecting"), connecting},
                    {QStringLiteral("ipAddress"), ipAddress},
                    {QStringLiteral("hasProfile"), false},
                });
            }
            continue;
        }

        if (baseDevice->type() != NetworkManager::Device::Wifi) {
            continue;
        }

        hasWirelessDevice = true;
        const auto device = baseDevice.objectCast<NetworkManager::WirelessDevice>();
        if (!device) {
            continue;
        }

        connect(device.data(), &NetworkManager::WirelessDevice::networkAppeared,
                this, &NetworkController::scheduleRebuild, Qt::UniqueConnection);
        connect(device.data(), &NetworkManager::WirelessDevice::networkDisappeared,
                this, &NetworkController::scheduleRebuild, Qt::UniqueConnection);

        QString activeSsid;
        if (const auto activeAccessPoint = device->activeAccessPoint()) {
            activeSsid = activeAccessPoint->ssid();
        }

        for (const auto &network : device->networks()) {
            if (!network || network->ssid().isEmpty() || !network->referenceAccessPoint()) {
                continue;
            }

            connect(network.data(), &NetworkManager::WirelessNetwork::signalStrengthChanged,
                    this, &NetworkController::scheduleRebuild, Qt::UniqueConnection);
            connect(network.data(), &NetworkManager::WirelessNetwork::referenceAccessPointChanged,
                    this, &NetworkController::scheduleRebuild, Qt::UniqueConnection);

            const auto accessPoint = network->referenceAccessPoint();
            const auto type = NetworkManager::findBestWirelessSecurity(device->wirelessCapabilities(),
                                                                       true,
                                                                       accessPoint->mode() == NetworkManager::AccessPoint::Adhoc,
                                                                       accessPoint->capabilities(),
                                                                       accessPoint->wpaFlags(),
                                                                       accessPoint->rsnFlags());

            NetworkEntry entry;
            entry.ssid = network->ssid();
            entry.signalStrength = network->signalStrength();
            entry.security = securityName(type);
            entry.secure = type != NetworkManager::NoneSecurity;
            entry.passwordRequired = type != NetworkManager::NoneSecurity && type != NetworkManager::OWE;
            entry.connected = entry.ssid == activeSsid;
            entry.devicePath = device->uni();
            entry.accessPointPath = accessPoint->uni();

            if (const auto connection = savedWirelessConnection(device, entry.ssid)) {
                entry.saved = true;
                entry.autoConnect = connection->settings() && connection->settings()->autoconnect();
            }
            entries.append(entry);
        }
    }

    std::sort(entries.begin(), entries.end(), [](const NetworkEntry &left, const NetworkEntry &right) {
        if (left.connected != right.connected) {
            return left.connected;
        }
        return left.signalStrength > right.signalStrength;
    });

    const auto key = [](const NetworkEntry &entry) {
        return entry.devicePath + QLatin1Char('\x1f') + entry.ssid;
    };

    for (int row = d->entries.size() - 1; row >= 0; --row) {
        const auto oldKey = key(d->entries.at(row));
        const bool stillPresent = std::any_of(entries.cbegin(), entries.cend(), [&key, &oldKey](const NetworkEntry &entry) {
            return key(entry) == oldKey;
        });
        if (!stillPresent) {
            beginRemoveRows({}, row, row);
            d->entries.removeAt(row);
            endRemoveRows();
        }
    }

    for (int targetRow = 0; targetRow < entries.size(); ++targetRow) {
        const auto targetKey = key(entries.at(targetRow));
        int currentRow = -1;
        for (int row = targetRow; row < d->entries.size(); ++row) {
            if (key(d->entries.at(row)) == targetKey) {
                currentRow = row;
                break;
            }
        }

        if (currentRow < 0) {
            beginInsertRows({}, targetRow, targetRow);
            d->entries.insert(targetRow, entries.at(targetRow));
            endInsertRows();
        } else {
            if (currentRow != targetRow) {
                beginMoveRows({}, currentRow, currentRow, {}, targetRow);
                d->entries.move(currentRow, targetRow);
                endMoveRows();
            }
            d->entries[targetRow] = entries.at(targetRow);
            Q_EMIT dataChanged(index(targetRow, 0), index(targetRow, 0));
        }
    }

    if (d->available != hasWirelessDevice) {
        d->available = hasWirelessDevice;
        Q_EMIT availableChanged();
    }

    if (d->wiredConnections != wiredConnections) {
        d->wiredConnections = wiredConnections;
        Q_EMIT wiredConnectionsChanged();
    }
}

void NetworkController::scheduleRebuild()
{
    d->rebuildTimer->start();
}

void NetworkController::setScanning(bool scanning)
{
    if (d->scanning == scanning) {
        return;
    }
    d->scanning = scanning;
    Q_EMIT scanningChanged();
}

void NetworkController::setErrorMessage(const QString &message)
{
    if (d->errorMessage == message) {
        return;
    }
    d->errorMessage = message;
    Q_EMIT errorMessageChanged();
}
