#include "networkcontroller.h"

#include <handler.h>

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QTimer>
#include <QEventLoop>
#include <QProcess>
#include <QSettings>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QHostAddress>
#include <QHostInfo>
#include <QAbstractSocket>

#include <QSet>
#include <QHash>
#include <QDebug>

#include <algorithm>
#include <functional>
#include <memory>

#include <NetworkManagerQt/AccessPoint>
#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/SecretAgent>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Setting>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessNetwork>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/VpnConnection>
#include <NetworkManagerQt/VpnSetting>
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
    QString connectionUuid;
    QString devicePath;
    QString accessPointPath;
};

void scheduleDnsResolutionTest(QObject *context,
                               const std::function<void()> &onFailure = {},
                               int attempt = 0)
{
    constexpr int maximumAttempts = 5;
    QTimer::singleShot(2000, context, [context, onFailure, attempt] {
        QHostInfo::lookupHost(QStringLiteral("example.com"), context, [context, onFailure, attempt](const QHostInfo &info) {
            if (info.error() == QHostInfo::NoError)
                return;
            if (attempt + 1 < maximumAttempts) {
                scheduleDnsResolutionTest(context, onFailure, attempt + 1);
            } else if (onFailure) {
                onFailure();
            }
        });
    });
}

void scheduleDnsResolutionTestAfterActivation(QObject *context,
                                              const QString &activeConnectionPath,
                                              const std::function<void()> &onFailure = {},
                                              int attempt = 0)
{
    constexpr int maximumAttempts = 100;
    const auto activeConnection = NetworkManager::findActiveConnection(activeConnectionPath);
    if (activeConnection && activeConnection->state() == NetworkManager::ActiveConnection::Activated) {
        scheduleDnsResolutionTest(context, onFailure);
        return;
    }
    if (activeConnection && activeConnection->state() == NetworkManager::ActiveConnection::Deactivated) {
        if (onFailure)
            onFailure();
        return;
    }
    if (attempt + 1 >= maximumAttempts) {
        if (onFailure)
            onFailure();
        return;
    }
    QTimer::singleShot(100, context, [context, activeConnectionPath, onFailure, attempt] {
        scheduleDnsResolutionTestAfterActivation(context, activeConnectionPath, onFailure, attempt + 1);
    });
}

bool disconnectActiveDevice(const QString &connectionUuid, const QString &devicePath, QString *errorMessage)
{
    for (const auto &active : NetworkManager::activeConnections()) {
        if (!active || active->uuid() != connectionUuid || !active->devices().contains(devicePath))
            continue;
        const auto device = NetworkManager::findNetworkInterface(devicePath);
        if (!device) {
            if (errorMessage)
                *errorMessage = QStringLiteral("The network device is no longer available.");
            return false;
        }
        auto reply = device->disconnectInterface();
        reply.waitForFinished();
        if (reply.isError()) {
            if (errorMessage)
                *errorMessage = reply.error().message();
            return false;
        }
        if (active->state() != NetworkManager::ActiveConnection::Deactivated) {
            QEventLoop eventLoop;
            QTimer timeout;
            timeout.setSingleShot(true);
            QObject::connect(active.data(),
                             &NetworkManager::ActiveConnection::stateChanged,
                             &eventLoop,
                             [&eventLoop](NetworkManager::ActiveConnection::State state) {
                                 if (state == NetworkManager::ActiveConnection::Deactivated)
                                     eventLoop.quit();
                             });
            QObject::connect(&timeout, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
            timeout.start(10000);
            eventLoop.exec();
        }
        if (active->state() != NetworkManager::ActiveConnection::Deactivated) {
            if (errorMessage)
                *errorMessage = QStringLiteral("Timed out waiting for the active connection to disconnect.");
            return false;
        }
        return true;
    }
    return true;
}

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

void sendNotification(const QString &title, const QString &message, const QString &iconName)
{
    const QString notifySend = QStandardPaths::findExecutable(QStringLiteral("notify-send"));
    if (notifySend.isEmpty()) {
        return;
    }

    QProcess::startDetached(notifySend,
                            {QStringLiteral("--app-name=System Message"),
                             QStringLiteral("--icon=%1").arg(iconName),
                             title,
                             message});
}
}

static void markVpnSecretsNotSaved(const NetworkManager::Connection::Ptr &connection,
                                   QObject *parent,
                                   std::function<void(bool, const QString &)> callback)
{
    if (!connection || !connection->settings()
        || connection->settings()->connectionType() != NetworkManager::ConnectionSettings::Vpn) {
        if (callback) {
            callback(false, QStringLiteral("The imported connection is not a VPN profile."));
        }
        return;
    }

    const auto settings = connection->settings();
    const auto vpnSetting = settings->setting(NetworkManager::Setting::Vpn).dynamicCast<NetworkManager::VpnSetting>();
    if (!vpnSetting) {
        if (callback) {
            callback(false, QStringLiteral("The imported VPN profile has no VPN settings."));
        }
        return;
    }

    NMStringMap data = vpnSetting->data();
    const QString notSaved = QString::number(static_cast<int>(NetworkManager::Setting::NotSaved));
    const QStringList secretFlagKeys = {
        QStringLiteral("password-flags"),
        QStringLiteral("proxy-password-flags"),
        QStringLiteral("http-proxy-password-flags"),
        QStringLiteral("cert-pass-flags"),
    };
    bool changed = false;
    for (const QString &key : secretFlagKeys) {
        if (data.value(key) != notSaved) {
            data.insert(key, notSaved);
            changed = true;
        }
    }
    const QStringList dataKeys = data.keys();
    for (const QString &key : dataKeys) {
        if (key.endsWith(QStringLiteral("-flags")) && data.value(key) != notSaved) {
            data.insert(key, notSaved);
            changed = true;
        }
    }

    NMVariantMapMap profile = settings->toMap();
    QVariantMap vpnMap = profile.value(QStringLiteral("vpn"));
    const bool hadSecrets = vpnMap.contains(QStringLiteral("secrets"));
    if (changed) {
        vpnSetting->setData(data);
        profile = settings->toMap();
        vpnMap = profile.value(QStringLiteral("vpn"));
    }
    vpnMap.remove(QStringLiteral("secrets"));
    profile.insert(QStringLiteral("vpn"), vpnMap);
    if (!changed && !hadSecrets) {
        if (callback) {
            callback(true, {});
        }
        return;
    }

    auto *watcher = new QDBusPendingCallWatcher(connection->update(profile), parent);
    const QString connectionName = connection->name();
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, parent, [watcher, connectionName, callback = std::move(callback)] {
        const QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            qWarning() << "Could not mark VPN secrets as non-persistent for" << connectionName << ":" << reply.error().message();
        }
        if (callback) {
            callback(!reply.isError(), reply.isError() ? reply.error().message() : QString());
        }
        watcher->deleteLater();
    });
}

class VpnSecretAgent final : public NetworkManager::SecretAgent
{
    Q_OBJECT

public:
    explicit VpnSecretAgent(QObject *parent = nullptr)
        : NetworkManager::SecretAgent(QStringLiteral("org.maui.settings.vpn"), NetworkManager::SecretAgent::VpnHints, parent)
    {
    }

    NMVariantMapMap GetSecrets(const NMVariantMapMap &connection,
                               const QDBusObjectPath &connectionPath,
                               const QString &settingName,
                               const QStringList &hints,
                               uint flags) override
    {
        Q_UNUSED(connection)
        Q_UNUSED(flags)

        setDelayedReply(true);
        const QDBusMessage callMessage = message();
        if (settingName != QStringLiteral("vpn")) {
            sendError(NoSecrets, QStringLiteral("No VPN secrets are available."), callMessage);
            return {};
        }

        const QString path = connectionPath.path();
        if (!m_expectedConnections.contains(path)) {
            sendError(NoSecrets, QStringLiteral("This VPN connection was not initiated by Maui Settings."), callMessage);
            return {};
        }

        const QString id = requestId(path, settingName);
        if (m_pendingRequests.contains(id)) {
            cancelPending(id, false);
        }

        m_pendingRequests.insert(id, PendingRequest{path, settingName, hints, callMessage});
        Q_EMIT secretsRequested(path, hints);
        return {};
    }

    void CancelGetSecrets(const QDBusObjectPath &connectionPath, const QString &settingName) override
    {
        cancelPending(requestId(connectionPath.path(), settingName), true);
    }

    void SaveSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath) override
    {
        Q_UNUSED(connection)
        Q_UNUSED(connectionPath)
        setDelayedReply(true);
        const QDBusMessage callMessage = message();
        sendError(NoSecrets, QStringLiteral("VPN secrets are configured not to be saved."), callMessage);
    }

    void DeleteSecrets(const NMVariantMapMap &connection, const QDBusObjectPath &connectionPath) override
    {
        Q_UNUSED(connection)
        Q_UNUSED(connectionPath)
        setDelayedReply(true);
        sendReply(message().createReply());
    }

    void expectConnection(const QString &connectionPath)
    {
        if (!connectionPath.isEmpty()) {
            m_expectedConnections.insert(connectionPath);
        }
    }

    void forgetConnection(const QString &connectionPath)
    {
        if (connectionPath.isEmpty()) {
            return;
        }

        m_expectedConnections.remove(connectionPath);
        cancelPending(requestId(connectionPath, QStringLiteral("vpn")), true);
    }

    void provide(const QString &connectionPath, const QVariantMap &secrets)
    {
        const QString id = requestId(connectionPath, QStringLiteral("vpn"));
        auto requestIt = m_pendingRequests.find(id);
        if (requestIt == m_pendingRequests.end()) {
            return;
        }

        const PendingRequest request = requestIt.value();
        NMStringMap values;
        for (auto it = secrets.cbegin(); it != secrets.cend(); ++it) {
            static const QStringList supportedKeys = {
                QStringLiteral("username"),
                QStringLiteral("password"),
                QStringLiteral("cert-pass"),
                QStringLiteral("proxy-password"),
                QStringLiteral("http-proxy-password"),
            };
            if (!supportedKeys.contains(it.key())) {
                continue;
            }

            const QString value = it.value().toString();
            if (!value.isEmpty()) {
                values.insert(it.key(), value);
            }
        }

        if (values.isEmpty()) {
            cancelPending(id, true);
            return;
        }

        m_pendingRequests.erase(requestIt);
        QVariantMap setting;
        setting.insert(QStringLiteral("name"), request.settingName);
        setting.insert(QStringLiteral("secrets"), QVariant::fromValue(values));

        NMVariantMapMap result;
        result.insert(request.settingName, setting);
        sendReply(request.message.createReply({QVariant::fromValue(result)}));
    }

    void cancel(const QString &connectionPath)
    {
        forgetConnection(connectionPath);
    }

Q_SIGNALS:
    void secretsRequested(const QString &connectionPath, const QStringList &hints);
    void secretsCanceled(const QString &connectionPath);

private:
    struct PendingRequest
    {
        QString connectionPath;
        QString settingName;
        QStringList hints;
        QDBusMessage message;
    };

    static QString requestId(const QString &connectionPath, const QString &settingName)
    {
        return connectionPath + QLatin1Char(0x0a) + settingName;
    }

    void cancelPending(const QString &id, bool notify)
    {
        const auto requestIt = m_pendingRequests.find(id);
        if (requestIt == m_pendingRequests.end()) {
            return;
        }

        const PendingRequest request = requestIt.value();
        m_pendingRequests.erase(requestIt);
        sendError(AgentCanceled, QStringLiteral("The VPN secret request was canceled."), request.message);
        if (notify) {
            Q_EMIT secretsCanceled(request.connectionPath);
        }
    }

    void sendReply(const QDBusMessage &reply) const
    {
        if (!QDBusConnection::systemBus().send(reply)) {
            qWarning() << "Failed to send VPN secret-agent reply:" << reply.errorMessage();
        }
    }

    QSet<QString> m_expectedConnections;
    QHash<QString, PendingRequest> m_pendingRequests;
};

class NetworkController::Private
{
public:
    QList<NetworkEntry> entries;
    bool available = false;
    bool scanning = false;
    int scanRequests = 0;
    QString errorMessage;
    QVariantList wiredConnections;
    QVariantList vpnConnections;
    QVariantMap connectedWirelessConnection;
    QTimer *rebuildTimer = nullptr;
    VpnSecretAgent *vpnSecretAgent = nullptr;
    QSet<QString> pendingVpnProfiles;
    QSet<QString> blockedVpnProfiles;
    QSet<QString> vpnImportBaseline;
    QString activeVpnSecretRequest;
    bool vpnImportInProgress = false;
    bool dnsEncryptionEnabled = false;
    quint64 dnsEncryptionGeneration = 0;
};

NetworkController::NetworkController(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private)
    , m_networkHandler(std::make_unique<Handler>(this))
{
    d->rebuildTimer = new QTimer(this);
    d->rebuildTimer->setSingleShot(true);
    d->rebuildTimer->setInterval(250);
    d->vpnSecretAgent = new VpnSecretAgent(this);
    connect(d->vpnSecretAgent, &VpnSecretAgent::secretsRequested, this, [this](const QString &connectionPath, const QStringList &hints) {
        const auto connection = NetworkManager::findConnection(connectionPath);
        if (connection && connection->settings()
            && connection->settings()->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
            if (!d->activeVpnSecretRequest.isEmpty() && d->activeVpnSecretRequest != connectionPath) {
                d->vpnSecretAgent->cancel(connectionPath);
                return;
            }
            d->activeVpnSecretRequest = connectionPath;
            Q_EMIT vpnSecretsRequested(connectionPath, connection->name(), hints);
        } else {
            d->vpnSecretAgent->cancel(connectionPath);
        }
    });
    connect(d->vpnSecretAgent, &VpnSecretAgent::secretsCanceled, this, [this](const QString &connectionPath) {
        if (d->activeVpnSecretRequest == connectionPath) {
            d->activeVpnSecretRequest.clear();
        }
        Q_EMIT vpnSecretsCanceled(connectionPath);
    });
    d->dnsEncryptionEnabled = QSettings().value(QStringLiteral("Network/dnsEncryptionEnabled"), false).toBool();
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

    auto *settingsNotifier = NetworkManager::settingsNotifier();
    connect(settingsNotifier, &NetworkManager::SettingsNotifier::connectionAdded, this, [this](const QString &) {
        scheduleRebuild();
    });
    connect(settingsNotifier, &NetworkManager::SettingsNotifier::connectionRemoved, this, [this](const QString &) {
        scheduleRebuild();
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
    case ConnectionUuidRole:
        return entry.connectionUuid;
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
        {ConnectionUuidRole, "connectionUuid"},
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

QVariantList NetworkController::vpnConnections() const
{
    return d->vpnConnections;
}

QVariantMap NetworkController::connectedWirelessConnection() const
{
    return d->connectedWirelessConnection;
}

bool NetworkController::dnsEncryptionEnabled() const
{
    return d->dnsEncryptionEnabled;
}

void NetworkController::setDnsEncryptionEnabled(bool enabled)
{
    const quint64 operation = ++d->dnsEncryptionGeneration;
    clearError();
    const auto notifyDnsToggleFailure = [this](const QString &message) {
        setErrorMessage(message);
        Q_EMIT dnsEncryptionEnabledChanged();
    };
    const auto scheduleToggleDnsTest = [this, enabled, operation](const QString &activeConnectionPath) {
        const auto onFailure = [this, enabled, operation] {
            if (!enabled || !d->dnsEncryptionEnabled || operation != d->dnsEncryptionGeneration)
                return;
            const QString message = tr("DNS resolution failed after enabling DNS encryption.");
            setDnsEncryptionEnabled(false);
            setErrorMessage(message);
        };
        scheduleDnsResolutionTestAfterActivation(this, activeConnectionPath, onFailure);
    };
    QSettings settings;
    const auto parseDns = [](const QString &value) {
        QList<QHostAddress> addresses;
        const auto values = value.split(QRegularExpression(QStringLiteral("[,[:space:]]+")), Qt::SkipEmptyParts);
        for (const auto &entry : values) {
            const QHostAddress address(entry.trimmed());
            if (!address.isNull())
                addresses.append(address);
        }
        return addresses;
    };
    const auto storedDns = [&settings, &parseDns](const QString &key) {
        const auto value = settings.value(key);
        QStringList values = value.toStringList();
        if (values.isEmpty() && !value.toString().isEmpty())
            values.append(value.toString());
        return parseDns(values.join(QStringLiteral(" ")));
    };
    const auto storedBool = [&settings](const QString &key, bool fallback) {
        const auto value = settings.value(key);
        if (!value.isValid())
            return fallback;
        return value.toBool() || value.toString().compare(QStringLiteral("yes"), Qt::CaseInsensitive) == 0;
    };

    struct DnsProfileRollback {
        NetworkManager::Connection::Ptr connection;
        NMVariantMapMap original;
        QList<QPair<QString, QString>> activeTargets;
    };
    QList<DnsProfileRollback> updatedProfiles;
    QSet<QString> createdBackups;
    QSet<QString> backupsToRemove;
    const auto rollbackProfiles = [&] {
        for (auto it = updatedProfiles.crbegin(); it != updatedProfiles.crend(); ++it) {
            auto restoreReply = it->connection->update(it->original);
            restoreReply.waitForFinished();
            if (const auto profileSettings = it->connection->settings())
                profileSettings->fromMap(it->original);
            for (const auto &target : it->activeTargets) {
                auto activationReply = NetworkManager::activateConnection(it->connection->path(), target.first, target.second);
                activationReply.waitForFinished();
            }
        }
        for (const auto &groupName : createdBackups)
            settings.remove(groupName);
    };

    const auto connections = NetworkManager::listConnections();

    for (const auto &connection : connections) {
        if (!connection || !connection->settings())
            continue;

        const auto persistedSettings = connection->settings();
        const auto type = persistedSettings->connectionType();
        if (type != NetworkManager::ConnectionSettings::Wired
            && type != NetworkManager::ConnectionSettings::Wireless) {
            continue;
        }

        const NMVariantMapMap originalMap = persistedSettings->toMap();
        const NetworkManager::ConnectionSettings::Ptr connectionSettings(new NetworkManager::ConnectionSettings(type));
        connectionSettings->fromMap(originalMap);
        const auto ipv4 = connectionSettings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
        const auto ipv6 = connectionSettings->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
        if (!ipv4 || !ipv6) {
            continue;
        }

        DnsProfileRollback profileRollback{connection, originalMap, {}};
        for (const auto &active : NetworkManager::activeConnections()) {
            if (!active || active->uuid() != connection->uuid() || active->devices().isEmpty())
                continue;
            const QString specificObject = type == NetworkManager::ConnectionSettings::Wireless ? active->specificObject() : QStringLiteral("/");
            profileRollback.activeTargets.append(QPair<QString, QString>{active->devices().constFirst(), specificObject});
        }

        const QString groupName = QStringLiteral("Network/DnsEncryption/") + connection->uuid();
        const bool hasDnsBackup = settings.contains(groupName + QStringLiteral("/ipv4Dns"))
            && settings.contains(groupName + QStringLiteral("/ipv6Dns"))
            && settings.contains(groupName + QStringLiteral("/ipv4IgnoreAutoDns"))
            && settings.contains(groupName + QStringLiteral("/ipv6IgnoreAutoDns"));
        const bool alreadyEncrypted = ipv4->ignoreAutoDns() && ipv6->ignoreAutoDns()
            && ipv4->dns().isEmpty() && ipv6->dns().isEmpty();
        if (enabled) {
            if (!hasDnsBackup && !alreadyEncrypted) {
                QStringList dns;
                for (const auto &address : ipv4->dns())
                    dns.append(address.toString());
                settings.setValue(groupName + QStringLiteral("/ipv4Dns"), dns);
                settings.setValue(groupName + QStringLiteral("/ipv6Dns"), [&ipv6] {
                    QStringList values;
                    for (const auto &address : ipv6->dns())
                        values.append(address.toString());
                    return values;
                }());
                settings.setValue(groupName + QStringLiteral("/ipv4IgnoreAutoDns"), ipv4->ignoreAutoDns());
                settings.setValue(groupName + QStringLiteral("/ipv6IgnoreAutoDns"), ipv6->ignoreAutoDns());
                createdBackups.insert(groupName);
            }
            ipv4->setDns({});
            ipv4->setIgnoreAutoDns(true);
            ipv6->setDns({});
            ipv6->setIgnoreAutoDns(true);
        } else if (hasDnsBackup) {
            settings.beginGroup(groupName);
            ipv4->setDns(storedDns(QStringLiteral("ipv4Dns")));
            ipv4->setIgnoreAutoDns(storedBool(QStringLiteral("ipv4IgnoreAutoDns"), false));
            ipv6->setDns(storedDns(QStringLiteral("ipv6Dns")));
            ipv6->setIgnoreAutoDns(storedBool(QStringLiteral("ipv6IgnoreAutoDns"), false));
            settings.endGroup();
        } else {
            if (ipv4->dns().isEmpty() && ipv4->ignoreAutoDns()) {
                ipv4->setIgnoreAutoDns(false);
            }
            if (ipv6->dns().isEmpty() && ipv6->ignoreAutoDns()) {
                ipv6->setIgnoreAutoDns(false);
            }
        }

        if (ipv4->method() != NetworkManager::Ipv4Setting::Manual
            && (!ipv4->addresses().isEmpty() || !ipv4->addressData().isEmpty())) {
            ipv4->setAddresses({});
            ipv4->setAddressData({});
            ipv4->setGateway(QString());
        }
        if (ipv6->method() != NetworkManager::Ipv6Setting::Manual
            && (!ipv6->addresses().isEmpty() || !ipv6->addressData().isEmpty())) {
            ipv6->setAddresses({});
            ipv6->setAddressData({});
        }


        NMVariantMapMap updateMap = connectionSettings->toMap();
        updateMap.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::Ipv4), ipv4->toMap());
        updateMap.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::Ipv6), ipv6->toMap());

        if (updateMap == profileRollback.original) {
            for (const auto &target : profileRollback.activeTargets) {
                QString deactivationError;
                if (!disconnectActiveDevice(connection->uuid(), target.first, &deactivationError)) {
                    notifyDnsToggleFailure(deactivationError);
                    return;
                }
                auto activationReply = NetworkManager::activateConnection(connection->path(), target.first, target.second);
                activationReply.waitForFinished();
                if (activationReply.isError()) {
                    notifyDnsToggleFailure(activationReply.error().message());
                    return;
                }
                scheduleToggleDnsTest(activationReply.value().path());
            }
            if (!enabled && hasDnsBackup)
                backupsToRemove.insert(groupName);
            continue;
        }

        updatedProfiles.append(profileRollback);
        auto updateReply = connection->update(updateMap);
        updateReply.waitForFinished();
        if (updateReply.isError()) {
            rollbackProfiles();
            notifyDnsToggleFailure(updateReply.error().message());
            return;
        }

        for (const auto &target : profileRollback.activeTargets) {
            QString deactivationError;
            if (!disconnectActiveDevice(connection->uuid(), target.first, &deactivationError)) {
                rollbackProfiles();
                notifyDnsToggleFailure(deactivationError);
                return;
            }
            auto activationReply = NetworkManager::activateConnection(connection->path(), target.first, target.second);
            activationReply.waitForFinished();
            if (activationReply.isError()) {
                rollbackProfiles();
                notifyDnsToggleFailure(activationReply.error().message());
                return;
            }
            scheduleToggleDnsTest(activationReply.value().path());
            break;
        }

        if (!enabled && hasDnsBackup)
            backupsToRemove.insert(groupName);
    }

    for (const auto &groupName : backupsToRemove)
        settings.remove(groupName);

    d->dnsEncryptionEnabled = enabled;
    settings.setValue(QStringLiteral("Network/dnsEncryptionEnabled"), enabled);
    Q_EMIT dnsEncryptionEnabledChanged();
    QTimer::singleShot(300, this, &NetworkController::rebuild);
}

void NetworkController::importOpenVpnConnection(const QString &filePath)
{
    clearError();
    if (filePath.trimmed().isEmpty()) {
        setErrorMessage(tr("Choose an OpenVPN configuration file."));
        return;
    }
    if (d->vpnImportInProgress) {
        setErrorMessage(tr("An OpenVPN connection import is already in progress."));
        return;
    }

    QSet<QString> existingVpnProfiles;
    for (const auto &connection : NetworkManager::listConnections()) {
        if (connection && connection->settings()
            && connection->settings()->connectionType() == NetworkManager::ConnectionSettings::Vpn) {
            existingVpnProfiles.insert(connection->path());
        }
    }

    d->vpnImportBaseline = existingVpnProfiles;
    d->vpnImportInProgress = true;

    QProcess process;
    process.start(QStringLiteral("nmcli"), {QStringLiteral("connection"), QStringLiteral("import"), QStringLiteral("type"), QStringLiteral("openvpn"), QStringLiteral("file"), filePath});
    process.waitForFinished(10000);
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        d->vpnImportInProgress = false;
        d->pendingVpnProfiles.clear();
        d->vpnImportBaseline.clear();
        setErrorMessage(QString::fromLocal8Bit(process.readAllStandardError()).trimmed());
        return;
    }

    QTimer::singleShot(300, this, [this, existingVpnProfiles] {
        QList<NetworkManager::Connection::Ptr> importedConnections;
        for (const auto &connection : NetworkManager::listConnections()) {
            if (connection && connection->settings()
                && connection->settings()->connectionType() == NetworkManager::ConnectionSettings::Vpn
                && !existingVpnProfiles.contains(connection->path())) {
                importedConnections.append(connection);
            }
        }

        if (importedConnections.isEmpty()) {
            d->vpnImportInProgress = false;
            d->vpnImportBaseline.clear();
            setErrorMessage(tr("NetworkManager did not create an OpenVPN connection profile."));
            rebuild();
            return;
        }

        d->pendingVpnProfiles.clear();
        for (const auto &connection : importedConnections) {
            d->pendingVpnProfiles.insert(connection->path());
        }

        for (const auto &connection : importedConnections) {
            const QString connectionPath = connection->path();
            markVpnSecretsNotSaved(connection, this, [this, connectionPath](bool success, const QString &errorMessage) {
                if (success) {
                    d->pendingVpnProfiles.remove(connectionPath);
                } else {
                    setErrorMessage(tr("Could not secure the imported VPN connection: %1").arg(errorMessage));
                    d->blockedVpnProfiles.insert(connectionPath);
                    d->pendingVpnProfiles.remove(connectionPath);
                    if (const auto importedConnection = NetworkManager::findConnection(connectionPath)) {
                        auto *watcher = new QDBusPendingCallWatcher(importedConnection->remove(), this);
                        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, connectionPath] {
                            const QDBusPendingReply<> reply = *watcher;
                            if (reply.isValid()) {
                                d->blockedVpnProfiles.remove(connectionPath);
                            } else {
                                qWarning() << "Could not remove an unprotected imported VPN connection:" << reply.error().message();
                            }
                            watcher->deleteLater();
                            rebuild();
                        });
                    }
                }

                if (d->pendingVpnProfiles.isEmpty()) {
                    d->vpnImportInProgress = false;
                    d->vpnImportBaseline.clear();
                }
                rebuild();
            });
        }
    });
}

void NetworkController::connectVpnConnection(const QString &connectionPath)
{
    clearError();
    const auto connection = NetworkManager::findConnection(connectionPath);
    if (!connection || !connection->settings() || connection->settings()->connectionType() != NetworkManager::ConnectionSettings::Vpn) {
        setErrorMessage(tr("The VPN connection profile is no longer available."));
        return;
    }
    if (!m_networkHandler || !m_networkHandler->canActivateConnection(connection->path())) {
        setErrorMessage(tr("NetworkManager does not support this VPN connection."));
        return;
    }
    if (!d->activeVpnSecretRequest.isEmpty() && d->activeVpnSecretRequest != connection->path()) {
        setErrorMessage(tr("Another VPN connection is waiting for credentials."));
        return;
    }

    d->vpnSecretAgent->expectConnection(connection->path());
    auto *watcher = new QDBusPendingCallWatcher(
        NetworkManager::activateConnection(connection->path(), QStringLiteral("/"), QStringLiteral("/")), this);
    const QString connectionName = connection->name();
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, connectionName, connectionPath] {
        const QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        if (reply.isError()) {
            d->vpnSecretAgent->forgetConnection(connectionPath);
            setErrorMessage(reply.error().message());
        } else {
            watchActivation(reply.value().path(), connectionName, QStringLiteral("network-vpn"));
        }
        watcher->deleteLater();
        QTimer::singleShot(300, this, &NetworkController::rebuild);
    });
}

void NetworkController::disconnectVpnConnection(const QString &connectionPath)
{
    clearError();
    const auto connection = NetworkManager::findConnection(connectionPath);
    if (!connection || !connection->settings() || connection->settings()->connectionType() != NetworkManager::ConnectionSettings::Vpn) {
        setErrorMessage(tr("The VPN connection profile is no longer available."));
        return;
    }

    d->vpnSecretAgent->forgetConnection(connectionPath);
    for (const auto &active : NetworkManager::activeConnections()) {
        if (!active || !active->vpn()) {
            continue;
        }

        const auto activeProfile = active->connection();
        if (!activeProfile || activeProfile->path() != connection->path()) {
            continue;
        }

        auto *watcher = new QDBusPendingCallWatcher(NetworkManager::deactivateConnection(active->path()), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
            const QDBusPendingReply<> reply = *watcher;
            if (reply.isError()) {
                setErrorMessage(reply.error().message());
            }
            watcher->deleteLater();
            QTimer::singleShot(300, this, &NetworkController::rebuild);
        });
        return;
    }
}

void NetworkController::submitVpnSecrets(const QString &connectionPath, const QVariantMap &secrets)
{
    if (d->vpnSecretAgent) {
        d->vpnSecretAgent->provide(connectionPath, secrets);
    }
}

void NetworkController::cancelVpnSecrets(const QString &connectionPath)
{
    if (d->vpnSecretAgent) {
        d->vpnSecretAgent->cancel(connectionPath);
    }
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
    const auto setConnectionError = [this, ssid](const QString &message) {
        if (message.contains(QStringLiteral("secret"), Qt::CaseInsensitive)
            || message.contains(QStringLiteral("password"), Qt::CaseInsensitive)
            || message.contains(QStringLiteral("auth"), Qt::CaseInsensitive)) {
            setErrorMessage(tr("The password for %1 is incorrect. Check it and try again.").arg(ssid));
        } else {
            setErrorMessage(message.isEmpty() ? tr("Could not connect to %1.").arg(ssid) : message);
        }
    };

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
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, ssid, setConnectionError] {
                const QDBusPendingReply<QDBusObjectPath> reply = *watcher;
                if (reply.isError()) {
                    setConnectionError(reply.error().message());
                } else {
                    watchActivation(reply.value().path(), ssid, QStringLiteral("network-wireless"));
                }
                watcher->deleteLater();
                QTimer::singleShot(300, this, &NetworkController::rebuild);
            });
            return;
        }
    }

    m_networkHandler->addAndActivateConnection(devicePath, accessPointPath, password);
    QTimer::singleShot(300, this, &NetworkController::rebuild);
}

void NetworkController::connectWired(const QString &devicePath, const QString &connectionPath)
{
    clearError();
    const auto connection = NetworkManager::findConnection(connectionPath);
    if (!connection) { setErrorMessage(tr("The wired connection profile is no longer available.")); return; }
    auto *watcher = new QDBusPendingCallWatcher(NetworkManager::activateConnection(connection->path(), devicePath, QStringLiteral("/")), this);
    const QString connectionName = connection->name();
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, connectionName] {
        const QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        if (reply.isError()) {
            setErrorMessage(reply.error().message());
        } else {
            watchActivation(reply.value().path(), connectionName, QStringLiteral("network-wired"));
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

QVariantMap NetworkController::connectionDetails(const QString &connectionUuid) const
{
    QVariantMap result;
    const auto addressList = [](const QList<NetworkManager::IpAddress> &addresses) {
        QStringList values;
        for (const auto &address : addresses) {
            if (!address.isValid())
                continue;
            values.append(QStringLiteral("%1/%2").arg(address.ip().toString()).arg(address.prefixLength()));
        }
        return values.join(QStringLiteral(" "));
    };
    const auto dnsList = [](const QList<QHostAddress> &dns) {
        QStringList values;
        for (const auto &address : dns)
            values.append(address.toString());
        return values.join(QStringLiteral(" "));
    };

    for (const auto &connection : NetworkManager::listConnections()) {
        if (!connection || connection->uuid() != connectionUuid || !connection->settings())
            continue;

        const auto settings = connection->settings();
        result.insert(QStringLiteral("profileName"), settings->id());
        result.insert(QStringLiteral("autoconnect"), settings->autoconnect());

        const auto ipv4 = settings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
        if (ipv4) {
            result.insert(QStringLiteral("ipv4Method"), ipv4->method() == NetworkManager::Ipv4Setting::Manual ? QStringLiteral("manual") : QStringLiteral("auto"));
            result.insert(QStringLiteral("ipv4Addresses"), addressList(ipv4->addresses()));
            result.insert(QStringLiteral("ipv4Gateway"), ipv4->gateway());
            result.insert(QStringLiteral("ipv4Dns"), dnsList(ipv4->dns()));
            result.insert(QStringLiteral("ipv4SearchDomains"), ipv4->dnsSearch().join(QStringLiteral(" ")));
            result.insert(QStringLiteral("ipv4IgnoreAutoDns"), ipv4->ignoreAutoDns());
        }

        const auto ipv6 = settings->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
        if (ipv6) {
            QString ipv6Method = QStringLiteral("auto");
            if (ipv6->method() == NetworkManager::Ipv6Setting::Manual)
                ipv6Method = QStringLiteral("manual");
            else if (ipv6->method() == NetworkManager::Ipv6Setting::ConfigDisabled)
                ipv6Method = QStringLiteral("disabled");
            result.insert(QStringLiteral("ipv6Method"), ipv6Method);
            result.insert(QStringLiteral("ipv6Addresses"), addressList(ipv6->addresses()));
            QString ipv6Gateway;
            for (const auto &address : ipv6->addresses()) {
                if (address.gateway().isNull())
                    continue;
                ipv6Gateway = address.gateway().toString();
                break;
            }
            result.insert(QStringLiteral("ipv6Gateway"), ipv6Gateway);
            result.insert(QStringLiteral("ipv6Dns"), dnsList(ipv6->dns()));
            result.insert(QStringLiteral("ipv6SearchDomains"), ipv6->dnsSearch().join(QStringLiteral(" ")));
            result.insert(QStringLiteral("ipv6IgnoreAutoDns"), ipv6->ignoreAutoDns());
        }
        break;
    }
    return result;
}

void NetworkController::updateConnection(const QString &connectionUuid, const QVariantMap &values)
{
    clearError();
    const quint64 dnsOperation = d->dnsEncryptionGeneration;
    const auto scheduleEditorDnsTest = [this, dnsOperation](const QString &activeConnectionPath) {
        const auto onFailure = [this, dnsOperation] {
            if (!d->dnsEncryptionEnabled || dnsOperation != d->dnsEncryptionGeneration)
                return;
            const QString message = tr("DNS resolution failed after saving the encrypted DNS connection.");
            setDnsEncryptionEnabled(false);
            setErrorMessage(message);
        };
        scheduleDnsResolutionTestAfterActivation(this, activeConnectionPath, onFailure);
    };
    if (connectionUuid.isEmpty()) {
        setErrorMessage(tr("The connection profile is no longer available."));
        return;
    }

    NetworkManager::Connection::Ptr connection;
    for (const auto &candidate : NetworkManager::listConnections()) {
        if (candidate && candidate->uuid() == connectionUuid) {
            connection = candidate;
            break;
        }
    }
    if (!connection || !connection->settings()) {
        setErrorMessage(tr("The connection profile is no longer available."));
        return;
    }

    const auto persistedSettings = connection->settings();
    const NMVariantMapMap originalMap = persistedSettings->toMap();
    const NetworkManager::ConnectionSettings::Ptr settings(new NetworkManager::ConnectionSettings(persistedSettings->connectionType()));
    settings->fromMap(originalMap);
    const auto ipv4 = settings->setting(NetworkManager::Setting::Ipv4).dynamicCast<NetworkManager::Ipv4Setting>();
    const auto ipv6 = settings->setting(NetworkManager::Setting::Ipv6).dynamicCast<NetworkManager::Ipv6Setting>();
    QList<QPair<QString, QString>> originalActiveTargets;
    for (const auto &active : NetworkManager::activeConnections()) {
        if (!active || active->uuid() != connectionUuid || active->devices().isEmpty())
            continue;
        const QString specificObject = settings->connectionType() == NetworkManager::ConnectionSettings::Wireless ? active->specificObject() : QStringLiteral("/");
        originalActiveTargets.append(QPair<QString, QString>{active->devices().constFirst(), specificObject});
    }
    QString createdDnsBackupGroup;
    const auto rollbackConnectionUpdate = [&] {
        auto rollbackReply = connection->update(originalMap);
        rollbackReply.waitForFinished();
        settings->fromMap(originalMap);
        if (const auto currentSettings = connection->settings())
            currentSettings->fromMap(originalMap);
        for (const auto &target : originalActiveTargets) {
            auto activationReply = NetworkManager::activateConnection(connection->path(), target.first, target.second);
            activationReply.waitForFinished();
        }
        if (!createdDnsBackupGroup.isEmpty()) {
            QSettings backupSettings;
            backupSettings.remove(createdDnsBackupGroup);
        }
    };
    const auto splitValues = [](const QString &value) {
        return value.split(QRegularExpression(QStringLiteral("[,[:space:]]+")), Qt::SkipEmptyParts);
    };
    const auto parseDns = [&splitValues](const QString &value) {
        QList<QHostAddress> addresses;
        for (const auto &entry : splitValues(value)) {
            const QHostAddress address(entry.trimmed());
            if (address.isNull())
                continue;
            addresses.append(address);
        }
        return addresses;
    };
    const auto parseAddresses = [&splitValues](const QString &value, bool *valid) {
        QList<NetworkManager::IpAddress> addresses;
        *valid = true;
        for (const auto &entry : splitValues(value)) {
            const int separator = entry.lastIndexOf(QLatin1String("/"));
            bool prefixValid = false;
            const QHostAddress address(entry.left(separator));
            const int prefix = entry.mid(separator + 1).toInt(&prefixValid);
            const int maxPrefix = address.protocol() == QAbstractSocket::IPv6Protocol ? 128 : 32;
            if (separator <= 0 || address.isNull() || !prefixValid || prefix < 0 || prefix > maxPrefix) {
                *valid = false;
                return QList<NetworkManager::IpAddress>();
            }
            NetworkManager::IpAddress value;
            value.setIp(address);
            value.setPrefixLength(prefix);
            addresses.append(value);
        }
        return addresses;
    };

    const auto dnsInputValid = [&splitValues](const QString &value, QAbstractSocket::NetworkLayerProtocol protocol) {
        for (const auto &entry : splitValues(value)) {
            const QHostAddress address(entry.trimmed());
            if (address.isNull() || address.protocol() != protocol)
                return false;
        }
        return true;
    };
    if (values.contains(QStringLiteral("ipv4Dns"))
        && !dnsInputValid(values.value(QStringLiteral("ipv4Dns")).toString(), QAbstractSocket::IPv4Protocol)) {
        setErrorMessage(tr("Enter IPv4 DNS servers as IPv4 addresses."));
        return;
    }
    if (values.contains(QStringLiteral("ipv6Dns"))
        && !dnsInputValid(values.value(QStringLiteral("ipv6Dns")).toString(), QAbstractSocket::IPv6Protocol)) {
        setErrorMessage(tr("Enter IPv6 DNS servers as IPv6 addresses."));
        return;
    }

    if (values.contains(QStringLiteral("id")))
        settings->setId(values.value(QStringLiteral("id")).toString());
    if (values.contains(QStringLiteral("autoconnect")))
        settings->setAutoconnect(values.value(QStringLiteral("autoconnect")).toBool());

    bool ipv4Manual = ipv4 && ipv4->method() == NetworkManager::Ipv4Setting::Manual;
    if (ipv4) {
        if (values.contains(QStringLiteral("ipv4Method"))) {
            const auto method = values.value(QStringLiteral("ipv4Method")).toString();
            ipv4Manual = method == QStringLiteral("manual");
            ipv4->setMethod(ipv4Manual ? NetworkManager::Ipv4Setting::Manual : NetworkManager::Ipv4Setting::Automatic);
        }
        if (values.contains(QStringLiteral("ipv4Addresses")) || (values.contains(QStringLiteral("ipv4Method")) && !ipv4Manual)) {
            if (ipv4Manual) {
                bool valid = false;
                const auto addresses = parseAddresses(values.value(QStringLiteral("ipv4Addresses")).toString(), &valid);
                if (!valid) {
                    setErrorMessage(tr("Enter IPv4 addresses using address/prefix notation."));
                    settings->fromMap(originalMap);
                    return;
                }
                ipv4->setAddresses(addresses);
                ipv4->setAddressData({});
            } else {
                ipv4->setAddresses({});
                ipv4->setAddressData({});
            }
        }
        if (values.contains(QStringLiteral("ipv4Gateway")) || (values.contains(QStringLiteral("ipv4Method")) && !ipv4Manual))
            ipv4->setGateway(ipv4Manual ? values.value(QStringLiteral("ipv4Gateway")).toString() : QString());
        if (values.contains(QStringLiteral("ipv4Dns")))
            ipv4->setDns(parseDns(values.value(QStringLiteral("ipv4Dns")).toString()));
        if (values.contains(QStringLiteral("ipv4SearchDomains")))
            ipv4->setDnsSearch(splitValues(values.value(QStringLiteral("ipv4SearchDomains")).toString()));
        if (values.contains(QStringLiteral("ipv4IgnoreAutoDns")))
            ipv4->setIgnoreAutoDns(values.value(QStringLiteral("ipv4IgnoreAutoDns")).toBool());
    }

    bool ipv6Manual = ipv6 && ipv6->method() == NetworkManager::Ipv6Setting::Manual;
    if (ipv6) {
        if (values.contains(QStringLiteral("ipv6Method"))) {
            const auto method = values.value(QStringLiteral("ipv6Method")).toString();
            ipv6Manual = method == QStringLiteral("manual");
            ipv6->setMethod(ipv6Manual
                                ? NetworkManager::Ipv6Setting::Manual
                                : (method == QStringLiteral("disabled") ? NetworkManager::Ipv6Setting::ConfigDisabled : NetworkManager::Ipv6Setting::Automatic));
        }
        if (values.contains(QStringLiteral("ipv6Addresses")) || (values.contains(QStringLiteral("ipv6Method")) && !ipv6Manual)) {
            QList<NetworkManager::IpAddress> addresses;
            if (ipv6Manual) {
                bool valid = false;
                addresses = parseAddresses(values.value(QStringLiteral("ipv6Addresses")).toString(), &valid);
                if (!valid) {
                    setErrorMessage(tr("Enter IPv6 addresses using address/prefix notation."));
                    settings->fromMap(originalMap);
                    return;
                }
            } else {
                addresses.clear();
            }
            if (values.contains(QStringLiteral("ipv6Gateway")) && ipv6Manual) {
                const QHostAddress gateway(values.value(QStringLiteral("ipv6Gateway")).toString());
                for (auto &address : addresses)
                    address.setGateway(gateway);
            }
            ipv6->setAddresses(addresses);
            ipv6->setAddressData({});
        } else if (values.contains(QStringLiteral("ipv6Gateway")) && ipv6Manual) {
            const QHostAddress gateway(values.value(QStringLiteral("ipv6Gateway")).toString());
            auto addresses = ipv6->addresses();
            for (auto &address : addresses)
                address.setGateway(gateway);
            ipv6->setAddresses(addresses);
            ipv6->setAddressData({});
        }
        if (values.contains(QStringLiteral("ipv6Dns")))
            ipv6->setDns(parseDns(values.value(QStringLiteral("ipv6Dns")).toString()));
        if (values.contains(QStringLiteral("ipv6SearchDomains")))
            ipv6->setDnsSearch(splitValues(values.value(QStringLiteral("ipv6SearchDomains")).toString()));
        if (values.contains(QStringLiteral("ipv6IgnoreAutoDns")))
            ipv6->setIgnoreAutoDns(values.value(QStringLiteral("ipv6IgnoreAutoDns")).toBool());
    }

    if (d->dnsEncryptionEnabled && ipv4 && ipv6
        && (settings->connectionType() == NetworkManager::ConnectionSettings::Wired
            || settings->connectionType() == NetworkManager::ConnectionSettings::Wireless)) {
        QSettings dnsSettings;
        const QString groupName = QStringLiteral("Network/DnsEncryption/") + connectionUuid;
        const bool hasDnsBackup = dnsSettings.contains(groupName + QStringLiteral("/ipv4Dns"))
            && dnsSettings.contains(groupName + QStringLiteral("/ipv6Dns"))
            && dnsSettings.contains(groupName + QStringLiteral("/ipv4IgnoreAutoDns"))
            && dnsSettings.contains(groupName + QStringLiteral("/ipv6IgnoreAutoDns"));
        const bool alreadyEncrypted = ipv4->ignoreAutoDns() && ipv6->ignoreAutoDns()
            && ipv4->dns().isEmpty() && ipv6->dns().isEmpty();
        if (!hasDnsBackup && !alreadyEncrypted) {
            QStringList ipv4Dns;
            for (const auto &address : ipv4->dns())
                ipv4Dns.append(address.toString());
            QStringList ipv6Dns;
            for (const auto &address : ipv6->dns())
                ipv6Dns.append(address.toString());
            dnsSettings.setValue(groupName + QStringLiteral("/ipv4Dns"), ipv4Dns);
            dnsSettings.setValue(groupName + QStringLiteral("/ipv6Dns"), ipv6Dns);
            dnsSettings.setValue(groupName + QStringLiteral("/ipv4IgnoreAutoDns"), ipv4->ignoreAutoDns());
            dnsSettings.setValue(groupName + QStringLiteral("/ipv6IgnoreAutoDns"), ipv6->ignoreAutoDns());
            createdDnsBackupGroup = groupName;
        }
        ipv4->setDns({});
        ipv4->setIgnoreAutoDns(true);
        ipv6->setDns({});
        ipv6->setIgnoreAutoDns(true);
    }


    NMVariantMapMap updateMap = settings->toMap();
    if (ipv4)
        updateMap.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::Ipv4), ipv4->toMap());
    if (ipv6)
        updateMap.insert(NetworkManager::Setting::typeAsString(NetworkManager::Setting::Ipv6), ipv6->toMap());

    auto updateReply = connection->update(updateMap);
    updateReply.waitForFinished();
    if (updateReply.isError()) {
        rollbackConnectionUpdate();
        setErrorMessage(updateReply.error().message());
        return;
    }

    for (const auto &active : NetworkManager::activeConnections()) {
        if (!active || active->uuid() != connectionUuid || active->devices().isEmpty())
            continue;

        const auto type = settings->connectionType();
        const QString specificObject = type == NetworkManager::ConnectionSettings::Wireless ? active->specificObject() : QStringLiteral("/");
        const QString devicePath = active->devices().constFirst();
        QString deactivationError;
        if (!disconnectActiveDevice(connectionUuid, devicePath, &deactivationError)) {
            rollbackConnectionUpdate();
            setErrorMessage(deactivationError);
            return;
        }
        auto activationReply = NetworkManager::activateConnection(connection->path(), devicePath, specificObject);
        activationReply.waitForFinished();
        if (activationReply.isError()) {
            rollbackConnectionUpdate();
            setErrorMessage(activationReply.error().message());
            return;
        }
        scheduleEditorDnsTest(activationReply.value().path());
        break;
    }
    QTimer::singleShot(300, this, &NetworkController::rebuild);
}

void NetworkController::clearError()
{
    setErrorMessage({});
}

void NetworkController::rebuild()
{
    QList<NetworkEntry> entries;
    QVariantList wiredConnections;
    QVariantMap connectedWirelessConnection;
    bool hasWirelessDevice = false;
    QVariantList vpnConnections;
    QHash<QString, NetworkManager::VpnConnection::State> activeVpnStates;

    for (const auto &active : NetworkManager::activeConnections()) {
        if (!active || !active->vpn()) {
            continue;
        }

        const auto connection = active->connection();
        const auto vpnConnection = active.objectCast<NetworkManager::VpnConnection>();
        if (!connection || !vpnConnection) {
            continue;
        }

        const auto vpnState = vpnConnection->state();
        activeVpnStates.insert(connection->path(), vpnState);
        if (vpnState == NetworkManager::VpnConnection::Activated
            || vpnState == NetworkManager::VpnConnection::Failed
            || vpnState == NetworkManager::VpnConnection::Disconnected) {
            d->vpnSecretAgent->forgetConnection(connection->path());
            if (d->activeVpnSecretRequest == connection->path()) {
                d->activeVpnSecretRequest.clear();
            }
        }
        connect(vpnConnection.data(),
                &NetworkManager::VpnConnection::stateChanged,
                this,
                &NetworkController::scheduleRebuild,
                Qt::UniqueConnection);
    }

    for (const auto &connection : NetworkManager::listConnections()) {
        if (!connection || !connection->settings()
            || connection->settings()->connectionType() != NetworkManager::ConnectionSettings::Vpn) {
            continue;
        }
        if (d->pendingVpnProfiles.contains(connection->path())
            || d->blockedVpnProfiles.contains(connection->path())
            || (d->vpnImportInProgress && !d->vpnImportBaseline.contains(connection->path()))) {
            continue;
        }

        connect(connection.data(), &NetworkManager::Connection::updated,
                this, &NetworkController::scheduleRebuild, Qt::UniqueConnection);

        const auto state = activeVpnStates.value(connection->path(), NetworkManager::VpnConnection::Disconnected);
        const bool connected = state == NetworkManager::VpnConnection::Activated;
        const bool connecting = state == NetworkManager::VpnConnection::Prepare
            || state == NetworkManager::VpnConnection::NeedAuth
            || state == NetworkManager::VpnConnection::Connecting
            || state == NetworkManager::VpnConnection::GettingIpConfig;
        vpnConnections.append(QVariantMap{
            {QStringLiteral("connectionName"), connection->name()},
            {QStringLiteral("connectionPath"), connection->path()},
            {QStringLiteral("connectionUuid"), connection->uuid()},
            {QStringLiteral("connected"), connected},
            {QStringLiteral("connecting"), connecting},
        });
    }

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
                    {QStringLiteral("devicePath"), baseDevice->uni()},
                    {QStringLiteral("connectionName"), connection->name()},
                    {QStringLiteral("connected"), connected},
                    {QStringLiteral("connecting"), profileConnecting},
                    {QStringLiteral("ipAddress"), connected ? ipAddress : QString()},
                    {QStringLiteral("hasProfile"), true},
                    {QStringLiteral("connectionPath"), connection->path()},
                    {QStringLiteral("connectionUuid"), connection->settings() ? connection->settings()->uuid() : QString()},
                    {QStringLiteral("autoConnect"), connection->settings() && connection->settings()->autoconnect()},
                });
            }

            if (!hasWiredProfile) {
                wiredConnections.append(QVariantMap{
                    {QStringLiteral("interfaceName"), baseDevice->interfaceName()},
                    {QStringLiteral("devicePath"), baseDevice->uni()},
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

            if (const auto connection = savedWirelessConnection(device, entry.ssid)) {
                entry.saved = true;
                entry.autoConnect = connection->settings() && connection->settings()->autoconnect();
                entry.connectionUuid = connection->settings() ? connection->settings()->uuid() : QString();
            }
            entry.accessPointPath = accessPoint->uni();

            if (entry.connected) {
                connectedWirelessConnection = QVariantMap{
                    {QStringLiteral("ssid"), entry.ssid},
                    {QStringLiteral("security"), entry.security},
                    {QStringLiteral("signalStrength"), entry.signalStrength},
                    {QStringLiteral("devicePath"), entry.devicePath},
                    {QStringLiteral("accessPointPath"), entry.accessPointPath},
                    {QStringLiteral("connectionUuid"), entry.connectionUuid},
                    {QStringLiteral("saved"), entry.saved},
                    {QStringLiteral("autoConnect"), entry.autoConnect},
                };
            } else {
                entries.append(entry);
            }
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

    if (d->vpnConnections != vpnConnections) {
        d->vpnConnections = vpnConnections;
        Q_EMIT vpnConnectionsChanged();
    }

    if (d->connectedWirelessConnection != connectedWirelessConnection) {
        d->connectedWirelessConnection = connectedWirelessConnection;
        Q_EMIT connectedWirelessConnectionChanged();
    }
}

void NetworkController::watchActivation(const QString &activeConnectionPath,
                                         const QString &networkName,
                                         const QString &iconName,
                                         const QString &connectionPath,
                                         int attempt)
{
    const auto activeConnection = NetworkManager::findActiveConnection(activeConnectionPath);
    if (!activeConnection) {
        if (attempt < 10) {
            QTimer::singleShot(100, this, [this, activeConnectionPath, networkName, iconName, connectionPath, attempt] {
                watchActivation(activeConnectionPath, networkName, iconName, connectionPath, attempt + 1);
            });
        }
        return;
    }

    const auto removeConnection = [this, connectionPath, activeConnection] {
        if (connectionPath.isEmpty()) {
            return;
        }

        if (const auto connection = activeConnection->connection(); connection && connection->path() == connectionPath) {
            auto *watcher = new QDBusPendingCallWatcher(connection->remove(), this);
            connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher] {
                watcher->deleteLater();
                QTimer::singleShot(300, this, &NetworkController::rebuild);
            });
        }
    };

    const auto handled = std::make_shared<bool>(false);
    const auto handleState = [this, networkName, iconName, removeConnection, handled](NetworkManager::ActiveConnection::State state) {
        if (*handled) {
            return;
        }

        if (state == NetworkManager::ActiveConnection::Activated) {
            *handled = true;
            sendNotification(tr("Connection Successful"), tr("Connected to %1.").arg(networkName), iconName);
        } else if (state == NetworkManager::ActiveConnection::Deactivated) {
            *handled = true;
            removeConnection();
            if (iconName == QStringLiteral("network-wireless")) {
                setErrorMessage(tr("The password for %1 is incorrect. Check it and try again.").arg(networkName));
            } else {
                setErrorMessage(tr("Could not connect to %1.").arg(networkName));
            }
        }
    };

    connect(activeConnection.data(),
            &NetworkManager::ActiveConnection::stateChanged,
            this,
            handleState);
    handleState(activeConnection->state());
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

#include "networkcontroller.moc"
