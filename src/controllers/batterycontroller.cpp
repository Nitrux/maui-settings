#include "batterycontroller.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QtMath>

#include "mauikit_system_control.h"

namespace {
constexpr qint64 historyLifetime = 7 * 24 * 60 * 60;
constexpr qint64 minimumSampleInterval = 5 * 60;

QVariant dbusProperty(const QString &service,
                       const QString &path,
                       const QString &interfaceName,
                       const char *name,
                       const QDBusConnection &connection)
{
    QDBusInterface properties(service,
                               path,
                               QStringLiteral("org.freedesktop.DBus.Properties"),
                               connection);
    if (!properties.isValid())
        return {};

    const QDBusReply<QDBusVariant> reply = properties.call(
        QStringLiteral("Get"), interfaceName, QString::fromLatin1(name));
    return reply.isValid() ? reply.value().variant() : QVariant {};
}

QString technologyText(int value)
{
    switch (value) {
    case 1:
        return QStringLiteral("Lithium-ion");
    case 2:
        return QStringLiteral("Lithium-polymer");
    case 3:
        return QStringLiteral("Lithium iron phosphate");
    case 4:
        return QStringLiteral("Lead-acid");
    case 5:
        return QStringLiteral("Nickel-cadmium");
    case 6:
        return QStringLiteral("Nickel-metal hydride");
    default:
        return {};
    }
}

QString capacityLevelText(int value)
{
    switch (value) {
    case 1:
        return QStringLiteral("None");
    case 2:
        return QStringLiteral("Critical");
    case 3:
        return QStringLiteral("Low");
    case 4:
        return QStringLiteral("Normal");
    case 5:
        return QStringLiteral("High");
    case 6:
        return QStringLiteral("Full");
    default:
        return {};
    }
}

QString preferredBatteryPath(const QString &fallbackPath)
{
    const QStringList preferredPaths {
        QStringLiteral("/sys/class/power_supply/BAT0"),
        QStringLiteral("/sys/class/power_supply/BAT1")
    };

    for (const QString &path : preferredPaths) {
        QFile typeFile(path + QStringLiteral("/type"));
        if (!typeFile.open(QIODevice::ReadOnly))
            continue;

        if (QString::fromUtf8(typeFile.readAll()).trimmed() == QLatin1String("Battery"))
            return path;
    }

    return fallbackPath;
}
}

BatteryController::BatteryController(QObject *parent)
    : QObject(parent)
    , m_historyPath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
                    + QStringLiteral("/battery-history.json"))
{
    auto connection = QDBusConnection::systemBus();
    const QString upowerService = QStringLiteral("org.freedesktop.UPower");
    const QString propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
    const QString upowerInterface = QStringLiteral("org.freedesktop.UPower");
    const QString upowerPath = QStringLiteral("/org/freedesktop/UPower");

    connection.connect(upowerService,
                       QString(),
                       propertiesInterface,
                       "PropertiesChanged",
                       this,
                       SLOT(handlePropertiesChanged(QString,QVariantMap,QStringList)));
    connection.connect(upowerService,
                       upowerPath,
                       upowerInterface,
                       "Changed",
                       this,
                       SLOT(handleUpowerChanged()));
    connection.connect(upowerService,
                       upowerPath,
                       upowerInterface,
                       "DeviceAdded",
                       this,
                       SLOT(handleDeviceChanged(QDBusObjectPath)));
    connection.connect(upowerService,
                       upowerPath,
                       upowerInterface,
                       "DeviceRemoved",
                       this,
                       SLOT(handleDeviceChanged(QDBusObjectPath)));

    loadHistory();
    refresh();
}

void BatteryController::handlePropertiesChanged(const QString &interfaceName,
                                             const QVariantMap &changedProperties,
                                             const QStringList &invalidatedProperties)
{
    Q_UNUSED(changedProperties)
    Q_UNUSED(invalidatedProperties)
    if (interfaceName == QStringLiteral("org.freedesktop.UPower.Device"))
        refresh();
}

void BatteryController::handleUpowerChanged()
{
    refresh();
}

void BatteryController::handleDeviceChanged(const QDBusObjectPath &devicePath)
{
    Q_UNUSED(devicePath)
    refresh();
}

bool BatteryController::available() const { return m_available; }
bool BatteryController::hasBattery() const { return m_hasBattery; }
int BatteryController::percentage() const { return m_percentage; }
QString BatteryController::state() const { return m_state; }
bool BatteryController::charging() const { return m_charging; }
bool BatteryController::pluggedIn() const { return m_pluggedIn; }
bool BatteryController::onBattery() const { return m_onBattery; }
qint64 BatteryController::timeToEmpty() const { return m_timeToEmpty; }
qint64 BatteryController::timeToFull() const { return m_timeToFull; }
int BatteryController::health() const { return m_health; }
QString BatteryController::capacityLevel() const { return m_capacityLevel; }
QString BatteryController::technology() const { return m_technology; }
double BatteryController::energy() const { return m_energy; }
double BatteryController::energyFull() const { return m_energyFull; }
double BatteryController::energyFullDesign() const { return m_energyFullDesign; }
double BatteryController::energyRate() const { return m_energyRate; }
double BatteryController::voltage() const { return m_voltage; }
int BatteryController::chargeCycles() const { return m_chargeCycles; }
QString BatteryController::vendor() const { return m_vendor; }
QString BatteryController::model() const { return m_model; }
QVariantList BatteryController::history() const { return historyVariantList(); }

void BatteryController::refresh()
{
    const QDBusConnection connection = QDBusConnection::systemBus();
    const QString upowerService = QStringLiteral("org.freedesktop.UPower");
    const QString upowerPath = QStringLiteral("/org/freedesktop/UPower");
    const QString upowerInterface = QStringLiteral("org.freedesktop.UPower");

    QDBusInterface upower(upowerService,
                          upowerPath,
                          upowerInterface,
                          connection);

    if (!upower.isValid()) {
        const bool changed = m_available || m_hasBattery;
        m_available = false;
        m_hasBattery = false;
        if (changed)
            Q_EMIT dataChanged();
        return;
    }

    QString batteryPath;
    bool mainsOnline = false;
    const bool systemBatteryAvailable = MauiKitSystem::batteryPowerSupplyState(&batteryPath, &mainsOnline);
    batteryPath = preferredBatteryPath(batteryPath);

    QString capacityText;
    QString statusText;
    if (systemBatteryAvailable)
        MauiKitSystem::readBatteryCharge(batteryPath, &capacityText, &statusText);

    const QString normalizedStatus = statusText.trimmed().toLower();
    const bool statusImpliesAc = normalizedStatus == QLatin1String("charging")
        || normalizedStatus == QLatin1String("full")
        || normalizedStatus == QLatin1String("not charging");

    m_onBattery = !(mainsOnline || statusImpliesAc);

    const QDBusReply<QDBusObjectPath> displayDevice = upower.call(QStringLiteral("GetDisplayDevice"));
    if (!displayDevice.isValid() || displayDevice.value().path() == QLatin1String("/")) {
        const bool changed = m_available || m_hasBattery;
        m_available = true;
        m_hasBattery = false;
        if (changed)
            Q_EMIT dataChanged();
        return;
    }

    const QString devicePath = displayDevice.value().path();
    const QString deviceInterface = QStringLiteral("org.freedesktop.UPower.Device");
    QString detailPath = devicePath;
    QStringList detailCandidates;

    const QDBusReply<QList<QDBusObjectPath>> devices = upower.call(QStringLiteral("EnumerateDevices"));
    if (devices.isValid()) {
        for (const QDBusObjectPath &candidatePath : devices.value()) {
            const QString candidate = candidatePath.path();
            QDBusInterface candidateDevice(upowerService, candidate, deviceInterface, connection);
            if (!candidateDevice.isValid())
                continue;

            const bool candidateIsBattery = dbusProperty(upowerService, candidate, deviceInterface, "Type", connection).toInt() == 2;
            const bool candidateIsPresent = dbusProperty(upowerService, candidate, deviceInterface, "IsPresent", connection).toBool();
            const bool candidatePowersSystem = dbusProperty(upowerService, candidate, deviceInterface, "PowerSupply", connection).toBool();
            if (candidateIsBattery && candidateIsPresent && candidatePowersSystem)
                detailCandidates.append(candidate);
        }
    }

    const QStringList preferredNames {QStringLiteral("BAT0"), QStringLiteral("BAT1")};
    for (const QString &preferredName : preferredNames) {
        for (const QString &candidate : detailCandidates) {
            const QString nativePath = dbusProperty(upowerService,
                                                     candidate,
                                                     deviceInterface,
                                                     "NativePath",
                                                     connection).toString();
            if (nativePath == preferredName || nativePath.endsWith(QLatin1Char('/') + preferredName)) {
                detailPath = candidate;
                break;
            }
        }

        if (detailPath != devicePath)
            break;
    }

    if (detailPath == devicePath && !detailCandidates.isEmpty())
        detailPath = detailCandidates.constFirst();

    QDBusInterface device(upowerService, devicePath, deviceInterface, connection);
    if (!device.isValid()) {
        const bool changed = m_available || m_hasBattery;
        m_available = true;
        m_hasBattery = false;
        if (changed)
            Q_EMIT dataChanged();
        return;
    }

    const auto property = [&devicePath, &deviceInterface, &upowerService, &connection](const char *name) {
        return dbusProperty(upowerService, devicePath, deviceInterface, name, connection);
    };
    const auto detailProperty = [&detailPath, &deviceInterface, &upowerService, &connection](const char *name) {
        return dbusProperty(upowerService, detailPath, deviceInterface, name, connection);
    };

    const QVariant type = property("Type");
    const QVariant present = property("IsPresent");
    const bool hasBattery = systemBatteryAvailable && type.toInt() == 2 && present.toBool();

    m_available = true;
    m_hasBattery = hasBattery;
    if (!hasBattery) {
        Q_EMIT dataChanged();
        return;
    }

    m_percentage = MauiKitSystem::parseBatteryPercent(capacityText);
    m_pluggedIn = !m_onBattery;

    double health = detailProperty("Capacity").toDouble();
    if (health <= 0.0)
        health = property("Capacity").toDouble();

    m_energy = qMax(0.0, property("Energy").toDouble());
    m_energyFull = qMax(0.0, property("EnergyFull").toDouble());
    m_energyFullDesign = qMax(0.0, detailProperty("EnergyFullDesign").toDouble());
    if (health <= 0.0 && m_energyFullDesign > 0.0) {
        const double detailEnergyFull = detailProperty("EnergyFull").toDouble();
        if (detailEnergyFull > 0.0)
            health = (detailEnergyFull / m_energyFullDesign) * 100.0;
    }
    m_health = qBound(0, qRound(health), 100);
    m_capacityLevel = capacityLevelText(detailProperty("CapacityLevel").toInt());
    m_technology = technologyText(detailProperty("Technology").toInt());
    m_energyRate = qMax(0.0, property("EnergyRate").toDouble());
    m_voltage = qMax(0.0, detailProperty("Voltage").toDouble());
    if (m_voltage <= 0.0)
        m_voltage = qMax(0.0, property("Voltage").toDouble());

    const QVariant chargeCycles = detailProperty("ChargeCycles");
    m_chargeCycles = chargeCycles.isValid() && !chargeCycles.isNull()
        ? chargeCycles.toInt()
        : -1;
    if (m_chargeCycles <= 0)
        m_chargeCycles = -1;

    m_vendor = detailProperty("Vendor").toString();
    if (m_vendor.isEmpty())
        m_vendor = property("Vendor").toString();
    m_model = detailProperty("Model").toString();
    if (m_model.isEmpty())
        m_model = property("Model").toString();

    const int stateValue = property("State").toInt();
    m_charging = normalizedStatus == QLatin1String("charging") || stateValue == 1;
    switch (stateValue) {
    case 1:
        m_state = QStringLiteral("Charging");
        break;
    case 2:
        m_state = QStringLiteral("Discharging");
        break;
    case 4:
        m_state = QStringLiteral("Fully charged");
        break;
    case 3:
        m_state = QStringLiteral("Empty");
        break;
    default:
        m_state = QStringLiteral("Unknown");
        break;
    }

    m_timeToEmpty = qMax<qint64>(0, property("TimeToEmpty").toLongLong());
    m_timeToFull = qMax<qint64>(0, property("TimeToFull").toLongLong());

    const bool historyWasChanged = addHistorySample(QDateTime::currentSecsSinceEpoch());
    Q_EMIT dataChanged();
    if (historyWasChanged)
        Q_EMIT historyChanged();
}

void BatteryController::loadHistory()
{
    QFile file(m_historyPath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isArray())
        return;

    for (const QJsonValue &value : document.array()) {
        const QJsonObject object = value.toObject();
        HistoryEntry entry;
        entry.timestamp = object.value(QStringLiteral("timestamp")).toVariant().toLongLong();
        entry.percentage = qBound(0, object.value(QStringLiteral("percentage")).toInt(), 100);
        entry.charging = object.value(QStringLiteral("charging")).toBool();
        entry.dischargeRate = qMax(0.0, object.value(QStringLiteral("dischargeRate")).toDouble());
        if (entry.timestamp > 0)
            m_history.append(entry);
    }

    pruneHistory(QDateTime::currentSecsSinceEpoch());
}

void BatteryController::saveHistory() const
{
    const QFileInfo fileInfo(m_historyPath);
    QDir().mkpath(fileInfo.path());

    QJsonArray array;
    for (const HistoryEntry &entry : m_history) {
        QJsonObject object;
        object.insert(QStringLiteral("timestamp"), entry.timestamp);
        object.insert(QStringLiteral("percentage"), entry.percentage);
        object.insert(QStringLiteral("charging"), entry.charging);
        object.insert(QStringLiteral("dischargeRate"), entry.dischargeRate);
        array.append(object);
    }

    QFile file(m_historyPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

void BatteryController::pruneHistory(qint64 now)
{
    const qint64 oldest = now - historyLifetime;
    while (!m_history.isEmpty() && m_history.constFirst().timestamp < oldest)
        m_history.removeFirst();
}

bool BatteryController::addHistorySample(qint64 now)
{
    pruneHistory(now);

    if (!m_history.isEmpty()) {
        const HistoryEntry &last = m_history.constLast();
        const double dischargeRate = m_charging ? 0.0 : m_energyRate;
        if (now - last.timestamp < minimumSampleInterval && last.percentage == m_percentage
            && last.charging == m_charging
            && qAbs(last.dischargeRate - dischargeRate) < 0.1)
            return false;
    }

    m_history.append({now, m_percentage, m_charging, m_charging ? 0.0 : m_energyRate});
    saveHistory();
    return true;
}

QVariantList BatteryController::historyVariantList() const
{
    QVariantList result;
    result.reserve(m_history.size());
    for (const HistoryEntry &entry : m_history) {
        result.append(QVariantMap {
            {QStringLiteral("timestamp"), entry.timestamp * 1000},
            {QStringLiteral("percentage"), entry.percentage},
            {QStringLiteral("charging"), entry.charging},
            {QStringLiteral("dischargeRate"), entry.dischargeRate}
        });
    }
    return result;
}
