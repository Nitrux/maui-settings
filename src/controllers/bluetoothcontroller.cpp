// Copyright 2026 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bluetoothcontroller.h"

#include <BluezQt/Agent>
#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/InitManagerJob>
#include <BluezQt/Manager>
#include <BluezQt/PendingCall>

#include <algorithm>
#include <functional>
#include <utility>

#include <QDBusObjectPath>
#include <QRandomGenerator>
#include <QTimer>

namespace
{
struct BluetoothEntry {
    QString name;
    QString address;
    QString iconName;
    bool paired = false;
    bool connected = false;
    bool trusted = false;
    int signalStrength = 0;
    QString devicePath;
};
}

class PairingAgent : public BluezQt::Agent
{
public:
    using PromptHandler = std::function<void(const QString &, const QString &, bool)>;

    explicit PairingAgent(PromptHandler handler, QObject *parent = nullptr)
        : BluezQt::Agent(parent)
        , m_handler(std::move(handler))
    {
    }

    QDBusObjectPath objectPath() const override
    {
        return QDBusObjectPath(QStringLiteral("/org/maui/settings/agent"));
    }

    void requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request) override
    {
        const QString pin = randomCode();
        request.accept(pin);
        m_handler(device->name(), pin, false);
    }

    void displayPinCode(BluezQt::DevicePtr device, const QString &pinCode) override
    {
        m_handler(device->name(), pinCode, false);
    }

    void requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request) override
    {
        const QString passkey = randomCode();
        request.accept(passkey.toUInt());
        m_handler(device->name(), passkey, false);
    }

    void displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered) override
    {
        Q_UNUSED(entered)
        m_handler(device->name(), passkey, false);
    }

    void requestConfirmation(BluezQt::DevicePtr device,
                             const QString &passkey,
                             const BluezQt::Request<> &request) override
    {
        m_request = request;
        m_requestPending = true;
        m_handler(device->name(), passkey, true);
    }

    void requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request) override
    {
        m_request = request;
        m_requestPending = true;
        m_handler(device->name(), {}, true);
    }

    void authorizeService(BluezQt::DevicePtr device,
                          const QString &uuid,
                          const BluezQt::Request<> &request) override
    {
        Q_UNUSED(uuid)
        if (device->isPaired()) {
            request.accept();
            return;
        }
        m_request = request;
        m_requestPending = true;
        m_handler(device->name(), {}, true);
    }

    void cancel() override
    {
        if (m_requestPending) {
            m_request.cancel();
            m_requestPending = false;
        }
        m_handler({}, {}, false);
    }

    void respond(bool accepted)
    {
        if (m_requestPending) {
            accepted ? m_request.accept() : m_request.reject();
            m_requestPending = false;
        }
        m_handler({}, {}, false);
    }

private:
    static QString randomCode()
    {
        return QStringLiteral("%1").arg(QRandomGenerator::global()->bounded(1000000), 6, 10, QLatin1Char('0'));
    }

    PromptHandler m_handler;
    BluezQt::Request<> m_request;
    bool m_requestPending = false;
};

class BluetoothController::Private
{
public:
    explicit Private(BluetoothController *parent)
        : manager(new BluezQt::Manager(parent))
    {
    }

    BluezQt::Manager *manager;
    PairingAgent *agent = nullptr;
    QList<BluetoothEntry> entries;
    bool initialized = false;
    bool agentRegistrationRequested = false;
    QString errorMessage;
    QString pairingDeviceName;
    QString pairingCode;
    bool pairingPromptActive = false;
    bool pairingConfirmationRequired = false;
    QTimer *rebuildTimer = nullptr;
};

BluetoothController::BluetoothController(QObject *parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
    d->rebuildTimer = new QTimer(this);
    d->rebuildTimer->setSingleShot(true);
    d->rebuildTimer->setInterval(250);
    connect(d->rebuildTimer, &QTimer::timeout, this, &BluetoothController::rebuild);

    d->agent = new PairingAgent([this](const QString &deviceName, const QString &code, bool confirmationRequired) {
        setPairingPrompt(deviceName, code, confirmationRequired);
    }, this);

    connect(d->manager, &BluezQt::Manager::operationalChanged, this, [this] {
        if (d->manager->isOperational() && !d->agentRegistrationRequested) {
            d->agentRegistrationRequested = true;
            watchCall(d->manager->registerAgent(d->agent));
        } else if (!d->manager->isOperational()) {
            d->agentRegistrationRequested = false;
        }
        Q_EMIT availableChanged();
        rebuild();
    });
    connect(d->manager, &BluezQt::Manager::adapterAdded, this, [this] {
        Q_EMIT availableChanged();
        Q_EMIT poweredChanged();
        Q_EMIT adapterNameChanged();
        rebuild();
    });
    connect(d->manager, &BluezQt::Manager::adapterRemoved, this, [this] {
        Q_EMIT availableChanged();
        Q_EMIT poweredChanged();
        Q_EMIT adapterNameChanged();
        rebuild();
    });
    connect(d->manager, &BluezQt::Manager::adapterChanged, this, [this] {
        Q_EMIT poweredChanged();
        Q_EMIT discoveringChanged();
        Q_EMIT adapterNameChanged();
        rebuild();
    });
    connect(d->manager, &BluezQt::Manager::deviceAdded, this, [this] {
        scheduleRebuild();
    });
    connect(d->manager, &BluezQt::Manager::deviceRemoved, this, [this] {
        scheduleRebuild();
    });
    connect(d->manager, &BluezQt::Manager::deviceChanged, this, [this] {
        scheduleRebuild();
    });

    auto *job = d->manager->init();
    connect(job, &BluezQt::InitManagerJob::result, this, [this](BluezQt::InitManagerJob *finishedJob) {
        d->initialized = true;
        if (finishedJob->error()) {
            setErrorMessage(finishedJob->errorText());
        } else if (d->manager->isOperational() && !d->agentRegistrationRequested) {
            d->agentRegistrationRequested = true;
            watchCall(d->manager->registerAgent(d->agent));
        }
        Q_EMIT availableChanged();
        Q_EMIT poweredChanged();
        Q_EMIT discoveringChanged();
        Q_EMIT adapterNameChanged();
        rebuild();
    });
    job->start();
}

BluetoothController::~BluetoothController()
{
    delete d;
}

int BluetoothController::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->entries.size();
}

QVariant BluetoothController::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= d->entries.size()) {
        return {};
    }

    const auto &entry = d->entries.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return entry.name;
    case AddressRole:
        return entry.address;
    case IconNameRole:
        return entry.iconName;
    case PairedRole:
        return entry.paired;
    case ConnectedRole:
        return entry.connected;
    case TrustedRole:
        return entry.trusted;
    case SignalStrengthRole:
        return entry.signalStrength;
    case DevicePathRole:
        return entry.devicePath;
    default:
        return {};
    }
}

QHash<int, QByteArray> BluetoothController::roleNames() const
{
    return {
        {NameRole, "name"},
        {AddressRole, "address"},
        {IconNameRole, "iconName"},
        {PairedRole, "paired"},
        {ConnectedRole, "connected"},
        {TrustedRole, "trusted"},
        {SignalStrengthRole, "signalStrength"},
        {DevicePathRole, "devicePath"},
    };
}

bool BluetoothController::available() const
{
    return d->initialized && d->manager->isOperational() && !d->manager->adapters().isEmpty();
}

bool BluetoothController::powered() const
{
    for (const auto &adapter : d->manager->adapters()) {
        if (adapter->isPowered()) {
            return true;
        }
    }
    return false;
}

void BluetoothController::setPowered(bool powered)
{
    clearError();
    if (powered) {
        d->manager->setBluetoothBlocked(false);
    }

    for (const auto &adapter : d->manager->adapters()) {
        if (!powered && adapter->isDiscovering()) {
            watchCall(adapter->stopDiscovery());
        }
        watchCall(adapter->setPowered(powered));
    }
}

bool BluetoothController::discovering() const
{
    for (const auto &adapter : d->manager->adapters()) {
        if (adapter->isDiscovering()) {
            return true;
        }
    }
    return false;
}

QString BluetoothController::adapterName() const
{
    const auto adapter = d->manager->usableAdapter();
    if (adapter) {
        return adapter->name();
    }
    const auto adapters = d->manager->adapters();
    return adapters.isEmpty() ? QString() : adapters.constFirst()->name();
}

QString BluetoothController::errorMessage() const
{
    return d->errorMessage;
}

bool BluetoothController::pairingPromptActive() const
{
    return d->pairingPromptActive;
}

bool BluetoothController::pairingConfirmationRequired() const
{
    return d->pairingConfirmationRequired;
}

QString BluetoothController::pairingDeviceName() const
{
    return d->pairingDeviceName;
}

QString BluetoothController::pairingCode() const
{
    return d->pairingCode;
}

void BluetoothController::setDiscoveryEnabled(bool enabled)
{
    clearError();
    const auto adapter = d->manager->usableAdapter();
    if (!adapter) {
        setErrorMessage(tr("Turn on a Bluetooth adapter before scanning."));
        return;
    }
    watchCall(enabled ? adapter->startDiscovery() : adapter->stopDiscovery());
}

void BluetoothController::pairDevice(const QString &devicePath)
{
    clearError();
    const auto device = d->manager->deviceForUbi(devicePath);
    if (!device) {
        setErrorMessage(tr("The Bluetooth device is no longer available."));
        return;
    }
    auto *call = device->pair();
    connect(call, &BluezQt::PendingCall::finished, this, [this] {
        d->agent->respond(false);
    });
    watchCall(call, true);
}

void BluetoothController::unpairDevice(const QString &devicePath)
{
    clearError();
    const auto device = d->manager->deviceForUbi(devicePath);
    if (!device || !device->adapter()) {
        setErrorMessage(tr("The Bluetooth device is no longer available."));
        return;
    }
    watchCall(device->adapter()->removeDevice(device));
}

void BluetoothController::connectDevice(const QString &devicePath)
{
    clearError();
    const auto device = d->manager->deviceForUbi(devicePath);
    if (!device) {
        setErrorMessage(tr("The Bluetooth device is no longer available."));
        return;
    }
    watchCall(device->connectToDevice());
}

void BluetoothController::disconnectDevice(const QString &devicePath)
{
    clearError();
    const auto device = d->manager->deviceForUbi(devicePath);
    if (!device) {
        setErrorMessage(tr("The Bluetooth device is no longer available."));
        return;
    }
    watchCall(device->disconnectFromDevice());
}

void BluetoothController::updateDevice(const QString &devicePath,
                                       const QString &name,
                                       bool trusted)
{
    clearError();
    const auto device = d->manager->deviceForUbi(devicePath);
    if (!device) {
        setErrorMessage(tr("The Bluetooth device is no longer available."));
        return;
    }

    const QString trimmedName = name.trimmed();
    if (!trimmedName.isEmpty() && trimmedName != device->name()) {
        watchCall(device->setName(trimmedName));
    }
    if (trusted != device->isTrusted()) {
        watchCall(device->setTrusted(trusted));
    }
}

void BluetoothController::respondToPairingPrompt(bool accepted)
{
    d->agent->respond(accepted);
}

void BluetoothController::clearError()
{
    setErrorMessage({});
}

void BluetoothController::rebuild()
{
    QList<BluetoothEntry> entries;
    for (const auto &device : d->manager->devices()) {
        if (!device || device->remoteName().isEmpty()) {
            continue;
        }

        BluetoothEntry entry;
        entry.name = device->name().isEmpty() ? device->address() : device->name();
        entry.address = device->address();
        entry.iconName = device->icon();
        entry.paired = device->isPaired();
        entry.connected = device->isConnected();
        entry.trusted = device->isTrusted();
        entry.signalStrength = device->rssi();
        entry.devicePath = device->ubi();
        entries.append(entry);
    }

    std::sort(entries.begin(), entries.end(), [](const BluetoothEntry &left, const BluetoothEntry &right) {
        if (left.connected != right.connected) {
            return left.connected;
        }
        if (left.paired != right.paired) {
            return left.paired;
        }
        return left.signalStrength > right.signalStrength;
    });

    const auto key = [](const BluetoothEntry &entry) {
        return entry.devicePath;
    };

    for (int row = d->entries.size() - 1; row >= 0; --row) {
        const auto oldKey = key(d->entries.at(row));
        const bool stillPresent = std::any_of(entries.cbegin(), entries.cend(), [&key, &oldKey](const BluetoothEntry &entry) {
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
}

void BluetoothController::scheduleRebuild()
{
    d->rebuildTimer->start();
}

void BluetoothController::watchCall(QObject *object, bool ignoreCancellation)
{
    auto *call = qobject_cast<BluezQt::PendingCall *>(object);
    if (!call) {
        return;
    }

    connect(call, &BluezQt::PendingCall::finished, this, [this, ignoreCancellation](BluezQt::PendingCall *finishedCall) {
        const int error = finishedCall->error();
        const bool canceled = error == BluezQt::PendingCall::Canceled
                           || error == BluezQt::PendingCall::Rejected
                           || error == BluezQt::PendingCall::AuthenticationCanceled
                           || error == BluezQt::PendingCall::AuthenticationRejected;
        if (error && !(ignoreCancellation && canceled)) {
            setErrorMessage(finishedCall->errorText());
        }
        rebuild();
        Q_EMIT poweredChanged();
        Q_EMIT discoveringChanged();
        finishedCall->deleteLater();
    });
}

void BluetoothController::setErrorMessage(const QString &message)
{
    if (d->errorMessage == message) {
        return;
    }
    d->errorMessage = message;
    Q_EMIT errorMessageChanged();
}

void BluetoothController::setPairingPrompt(const QString &deviceName,
                                           const QString &code,
                                           bool confirmationRequired)
{
    d->pairingDeviceName = deviceName;
    d->pairingCode = code;
    d->pairingConfirmationRequired = confirmationRequired;
    d->pairingPromptActive = !deviceName.isEmpty();
    Q_EMIT pairingPromptChanged();
}
