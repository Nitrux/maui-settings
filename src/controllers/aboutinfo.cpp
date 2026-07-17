#include "aboutinfo.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QProcess>
#include <QRegularExpression>
#include <QStorageInfo>
#include <QStringList>
#include <QSysInfo>
#include <QTimer>
#include <QVariantMap>
#include <QList>
#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#endif

namespace
{
QString envValue(const char *name)
{
    return QString::fromUtf8(qgetenv(name)).trimmed();
}

QString firstExistingPath(const QStringList &paths)
{
    for (const QString &path : paths)
    {
        if (QFileInfo::exists(path))
            return path;
    }

    return {};
}

QString trimQuotedValue(const QString &line)
{
    const int equalsIndex = line.indexOf(QLatin1Char('='));
    if (equalsIndex < 0)
        return {};

    QString value = line.mid(equalsIndex + 1).trimmed();
    if (value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"')) && value.size() >= 2)
        value = value.mid(1, value.size() - 2);

    return value;
}

QString readKeyValue(const QString &path, const QString &key)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    const QString prefix = key + QLatin1Char('=');
    while (!file.atEnd())
    {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (!line.startsWith(prefix))
            continue;

        const QString value = trimQuotedValue(line);
        if (!value.isEmpty())
            return value;
    }

    return {};
}

QString readMemInfoValueKBFromPath(const QString &path, const QString &key)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    const QString prefix = key + QLatin1Char(':');
    while (!file.atEnd())
    {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (!line.startsWith(prefix))
            continue;

        QString value = line.section(QLatin1Char(':'), 1).trimmed();
        const int spaceIndex = value.indexOf(QLatin1Char(' '));
        if (spaceIndex > 0)
            value = value.left(spaceIndex);

        return value;
    }

    return {};
}

QString readCpuModelFromPath(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    while (!file.atEnd())
    {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (!line.toLower().contains(QStringLiteral("model name")))
            continue;

        const int separatorIndex = line.indexOf(QLatin1Char(':'));
        if (separatorIndex < 0)
            continue;

        const QString value = line.mid(separatorIndex + 1).trimmed();
        if (!value.isEmpty())
            return value;
    }

    return {};
}

QString readableDataSize(qint64 bytes)
{
    return QLocale().formattedDataSize(bytes);
}

QString formatSession(const QString &sessionType, const QString &desktop)
{
    if (sessionType.isEmpty() && desktop.isEmpty())
        return QStringLiteral("Unknown session");

    if (sessionType.isEmpty())
        return desktop;
    if (desktop.isEmpty())
        return sessionType;

    return QStringLiteral("%1 / %2").arg(sessionType, desktop);
}

#ifdef Q_OS_LINUX
qint64 systemMemoryTotalKB()
{
    struct sysinfo info;
    if (sysinfo(&info) != 0)
        return 0;

    return (static_cast<qint64>(info.totalram) * static_cast<qint64>(info.mem_unit)) / 1024;
}

qint64 systemMemoryAvailableKB()
{
    struct sysinfo info;
    if (sysinfo(&info) != 0)
        return 0;

    return (static_cast<qint64>(info.freeram) + static_cast<qint64>(info.bufferram)) * static_cast<qint64>(info.mem_unit) / 1024;
}
#endif

QString normalizeDevicePath(const QString &devicePath)
{
    if (devicePath.isEmpty())
        return {};

    const QFileInfo deviceInfo(devicePath);
    const QString canonical = deviceInfo.canonicalFilePath();
    return canonical.isEmpty() ? devicePath : canonical;
}

QString readSysfsText(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    return QString::fromUtf8(file.readAll()).trimmed();
}

qint64 readSysfsInteger(const QString &path)
{
    bool ok = false;
    const qint64 value = readSysfsText(path).toLongLong(&ok);
    return ok ? value : 0;
}

QString diskNameFromDevicePath(const QString &devicePath)
{
    return QFileInfo(devicePath).fileName();
}

QString diskNameFromPartitionName(const QString &partitionName)
{
    if (partitionName.startsWith(QStringLiteral("nvme")) || partitionName.startsWith(QStringLiteral("mmcblk")))
    {
        const int separatorIndex = partitionName.lastIndexOf(QLatin1Char('p'));
        if (separatorIndex > 0)
            return partitionName.left(separatorIndex);
    }
    else if (partitionName.startsWith(QStringLiteral("sd")) || partitionName.startsWith(QStringLiteral("hd")) || partitionName.startsWith(QStringLiteral("vd")) || partitionName.startsWith(QStringLiteral("xvd")))
    {
        int index = partitionName.size() - 1;
        while (index >= 0 && partitionName.at(index).isDigit())
            --index;

        if (index >= 0 && index < partitionName.size() - 1)
            return partitionName.left(index + 1);
    }

    return partitionName;
}

QString diskDevicePath(const QString &diskName)
{
    return QStringLiteral("/dev/") + diskName;
}

QString diskDisplayName(const QString &diskName)
{
    const QString basePath = QStringLiteral("/sys/class/block/") + diskName + QStringLiteral("/device/");
    const QString model = readSysfsText(basePath + QStringLiteral("model")).simplified();
    const QString vendor = readSysfsText(basePath + QStringLiteral("vendor")).simplified();

    if (!model.isEmpty())
        return model;
    if (!vendor.isEmpty())
        return QStringLiteral("%1 %2").arg(vendor, diskName);

    return diskName;
}

QString diskTypeLabel(const QString &diskName)
{
    if (diskName.startsWith(QStringLiteral("nvme")))
        return QStringLiteral("nvme");

    const QString removable = readSysfsText(QStringLiteral("/sys/class/block/") + diskName + QStringLiteral("/removable"));
    if (removable == QStringLiteral("1"))
        return QStringLiteral("usb");

    const QString rotational = readSysfsText(QStringLiteral("/sys/class/block/") + diskName + QStringLiteral("/queue/rotational"));
    if (rotational == QStringLiteral("1"))
        return QStringLiteral("hdd");
    if (rotational == QStringLiteral("0"))
        return QStringLiteral("ssd");

    return QStringLiteral("disk");
}

qint64 diskTotalBytes(const QString &diskName)
{
    return readSysfsInteger(QStringLiteral("/sys/class/block/") + diskName + QStringLiteral("/size")) * 512;
}

bool isGpuController(const QString &className)
{
    const QString lowerClass = className.toLower();
    return lowerClass.contains(QStringLiteral("vga compatible controller")) || lowerClass.contains(QStringLiteral("3d controller")) || lowerClass.contains(QStringLiteral("display controller"));
}

QString cleanGpuModel(const QString &description)
{
    QString model = description.trimmed();
    model.remove(QRegularExpression(QStringLiteral(R"(\s+\(rev [^)]+\)$)")));
    model.remove(QRegularExpression(QStringLiteral(R"(\s+\[[0-9a-fA-F]{4}:[0-9a-fA-F]{4}\]$)")));
    return model.simplified();
}

QVariantMap graphicsDeviceToMap(const QString &address, const QString &model, const QString &driver)
{
    const QString safeDriver = driver.isEmpty() ? QStringLiteral("Unknown driver") : driver;

    QVariantMap data;
    data.insert(QStringLiteral("title"), model.isEmpty() ? address : model);
    data.insert(QStringLiteral("address"), address);
    data.insert(QStringLiteral("driver"), safeDriver);
    data.insert(QStringLiteral("subtitle"), QStringLiteral("%1 · %2").arg(safeDriver, address));
    return data;
}

QVariantList graphicsDeviceList()
{
    QProcess process;
    process.start(QStringLiteral("lspci"), {QStringLiteral("-Dnnk")});
    if (!process.waitForFinished(3000) || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        return {};

    const QString output = QString::fromUtf8(process.readAllStandardOutput());
    const QStringList lines = output.split(QRegularExpression(QStringLiteral(R"(\r?\n)")), Qt::SkipEmptyParts);
    QVariantList devices;

    QString currentAddress;
    QString currentClass;
    QString currentDescription;
    QString currentDriver;

    const QRegularExpression headerRegex(QStringLiteral(R"(^([0-9a-fA-F:.]+)\s+([^:]+):\s*(.*)$)"));

    auto commit = [&]() {
        if (!currentAddress.isEmpty() && isGpuController(currentClass))
            devices.append(graphicsDeviceToMap(currentAddress, cleanGpuModel(currentDescription), currentDriver));

        currentAddress.clear();
        currentClass.clear();
        currentDescription.clear();
        currentDriver.clear();
    };

    for (const QString &line : lines)
    {
        if (line.startsWith(QLatin1Char(' ')) || line.startsWith(QLatin1Char('\t')))
        {
            if (line.contains(QStringLiteral("Kernel driver in use:")))
            {
                const QString driver = line.section(QLatin1Char(':'), 1).trimmed();
                if (!driver.isEmpty())
                    currentDriver = driver;
            }
            continue;
        }

        commit();

        const QRegularExpressionMatch match = headerRegex.match(line);
        if (!match.hasMatch())
            continue;

        currentAddress = match.captured(1);
        currentClass = match.captured(2);
        currentDescription = match.captured(3);
    }

    commit();
    return devices;
}

bool isPhysicalDisk(const QString &diskName)
{
    if (!QFileInfo::exists(QStringLiteral("/sys/class/block/") + diskName + QStringLiteral("/device")))
        return false;

    if (QFileInfo::exists(QStringLiteral("/sys/class/block/") + diskName + QStringLiteral("/partition")))
        return false;

    const QString uevent = readSysfsText(QStringLiteral("/sys/class/block/") + diskName + QStringLiteral("/uevent"));
    if (!uevent.isEmpty() && !uevent.contains(QStringLiteral("DEVTYPE=disk")))
        return false;

    return diskTotalBytes(diskName) > 0;
}

QString storageUsageSummary(qint64 usedBytes, qint64 totalBytes)
{
    const QString totalText = totalBytes > 0 ? readableDataSize(totalBytes) : QStringLiteral("Unavailable");
    const QString usedText = usedBytes >= 0 ? readableDataSize(usedBytes) : QStringLiteral("Unavailable");
    return QStringLiteral("%1 used of %2 total").arg(usedText, totalText);
}

QVariantMap storageDeviceToMap(const QString &diskName, const QList<QStorageInfo> &mountedVolumes)
{
    const QString devicePath = diskDevicePath(diskName);
    const qint64 totalBytes = diskTotalBytes(diskName);
    qint64 usedBytes = 0;
    bool hasUsage = false;
    QStringList mountPoints;
    QStringList seenPartitionDevices;

    for (const QStorageInfo &storage : mountedVolumes)
    {
        if (!storage.isValid() || !storage.isReady())
            continue;

        const QString partitionDevice = normalizeDevicePath(QString::fromUtf8(storage.device()));
        if (!partitionDevice.startsWith(QStringLiteral("/dev/")))
            continue;

        if (seenPartitionDevices.contains(partitionDevice))
            continue;
        seenPartitionDevices.append(partitionDevice);

        const QString partitionName = diskNameFromDevicePath(partitionDevice);
        if (diskNameFromPartitionName(partitionName) != diskName)
            continue;

        const QString mountPoint = storage.rootPath();
        if (!mountPoint.isEmpty() && !mountPoints.contains(mountPoint))
            mountPoints.append(mountPoint);

        const qint64 partitionTotal = storage.bytesTotal();
        const qint64 partitionAvailable = storage.bytesAvailable();
        if (partitionTotal > 0 && partitionAvailable >= 0)
        {
            usedBytes += qMax<qint64>(0, partitionTotal - partitionAvailable);
            hasUsage = true;
        }
    }

    mountPoints.sort();

    QVariantMap data;
    data.insert(QStringLiteral("title"), diskDisplayName(diskName));
    data.insert(QStringLiteral("devicePath"), devicePath);
    data.insert(QStringLiteral("deviceType"), diskTypeLabel(diskName));
    data.insert(QStringLiteral("mountPoints"), mountPoints.join(QStringLiteral(", ")));
    data.insert(QStringLiteral("subtitle"), mountPoints.isEmpty() ? QStringLiteral("%1 · %2").arg(devicePath, diskTypeLabel(diskName))
                                                                     : QStringLiteral("%1 · %2 · mounted at %3").arg(devicePath, diskTypeLabel(diskName), mountPoints.join(QStringLiteral(", "))));
    const bool hasTotal = totalBytes > 0;
    const bool showUsage = hasUsage && hasTotal;
    const qint64 displayUsedBytes = showUsage ? usedBytes : 0;
    const qint64 displayAvailableBytes = hasTotal ? qMax<qint64>(0, totalBytes - displayUsedBytes) : -1;

    data.insert(QStringLiteral("used"), showUsage ? readableDataSize(displayUsedBytes) : QStringLiteral("Unavailable"));
    data.insert(QStringLiteral("available"), showUsage ? readableDataSize(displayAvailableBytes) : QStringLiteral("Unavailable"));
    data.insert(QStringLiteral("total"), hasTotal ? readableDataSize(totalBytes) : QStringLiteral("Unavailable"));
    data.insert(QStringLiteral("usageSummary"), showUsage ? storageUsageSummary(displayUsedBytes, totalBytes) : QStringLiteral("Usage unavailable"));
    data.insert(QStringLiteral("usageFraction"), showUsage ? qBound(0.0, static_cast<double>(usedBytes) / static_cast<double>(totalBytes), 1.0) : 0.0);
    data.insert(QStringLiteral("indeterminate"), false);
    return data;
}

QVariantList storageDeviceList()
{
    const QList<QStorageInfo> mountedVolumes = QStorageInfo::mountedVolumes();
    const QStringList diskNames = QDir(QStringLiteral("/sys/class/block")).entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    QVariantList devices;
    for (const QString &diskName : diskNames)
    {
        if (!isPhysicalDisk(diskName))
            continue;

        devices.append(storageDeviceToMap(diskName, mountedVolumes));
    }

    return devices;
}
} // namespace

QVariantList AboutInfo::graphicsDeviceList()
{
    return ::graphicsDeviceList();
}

AboutInfo::AboutInfo(QObject *parent)
    : QObject(parent)
    , m_graphicsDevices(graphicsDeviceList())
{
    auto *timer = new QTimer(this);
    timer->setInterval(2000);
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, &AboutInfo::refreshStorageDevices);
    timer->start();

    refreshStorageDevices();
}

QString AboutInfo::formatBytes(qint64 bytes)
{
    return readableDataSize(bytes);
}

QString AboutInfo::readReleaseValue(const QString &path, const QString &key)
{
    return readKeyValue(path, key);
}

qint64 AboutInfo::readMemInfoValueKB(const QString &key)
{
    const QString value = readMemInfoValueKBFromPath(QStringLiteral("/proc/meminfo"), key);
    if (!value.isEmpty())
        return value.toLongLong();

#ifdef Q_OS_LINUX
    if (key == QStringLiteral("MemTotal"))
        return systemMemoryTotalKB();
    if (key == QStringLiteral("MemAvailable"))
        return systemMemoryAvailableKB();
#endif

    return 0;
}

QString AboutInfo::readCpuModel()
{
    QProcess process;
    process.start(QStringLiteral("grep"), {QStringLiteral("-m"), QStringLiteral("1"), QStringLiteral("model name"), QStringLiteral("/proc/cpuinfo")});
    if (process.waitForFinished(3000) && process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)
    {
        const QString output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        const int separatorIndex = output.indexOf(QLatin1Char(':'));
        if (separatorIndex >= 0)
        {
            const QString model = output.mid(separatorIndex + 1).trimmed();
            if (!model.isEmpty())
                return model;
        }
    }

    const QString path = firstExistingPath({QStringLiteral("/proc/cpuinfo")});
    return path.isEmpty() ? QString() : readCpuModelFromPath(path);
}

QString AboutInfo::osDistribution() const
{
    const QString releasePath = firstExistingPath({QStringLiteral("/etc/os-release"), QStringLiteral("/usr/lib/os-release")});
    const QString distribution = releasePath.isEmpty() ? QString() : readReleaseValue(releasePath, QStringLiteral("NAME"));
    return distribution.isEmpty() ? QStringLiteral("Unknown operating system") : distribution;
}

QString AboutInfo::osVersion() const
{
    const QString releasePath = firstExistingPath({QStringLiteral("/etc/os-release"), QStringLiteral("/usr/lib/os-release")});
    const QString version = releasePath.isEmpty() ? QString() : readReleaseValue(releasePath, QStringLiteral("VERSION"));
    return version.isEmpty() ? QStringLiteral("Unknown version") : version;
}

QString AboutInfo::osKernel() const
{
    return QStringLiteral("%1 %2").arg(QSysInfo::kernelType(), QSysInfo::kernelVersion());
}

QString AboutInfo::osSession() const
{
    return formatSession(envValue("XDG_SESSION_TYPE"), envValue("XDG_CURRENT_DESKTOP"));
}

QString AboutInfo::hostName() const
{
    const QString host = QSysInfo::machineHostName();
    return host.isEmpty() ? QStringLiteral("Unknown host") : host;
}

QString AboutInfo::cpuModel() const
{
    const QString model = readCpuModel();
    return model.isEmpty() ? QStringLiteral("Unknown CPU") : model;
}

QString AboutInfo::cpuArchitecture() const
{
    QProcess process;
    process.start(QStringLiteral("uname"), {QStringLiteral("-m")});
    if (!process.waitForFinished(3000) || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        return QStringLiteral("Unknown architecture");

    const QString architecture = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    return architecture.isEmpty() ? QStringLiteral("Unknown architecture") : architecture;
}

QVariantList AboutInfo::graphicsDevices() const
{
    return m_graphicsDevices;
}

QString AboutInfo::memoryTotal() const
{
    const qint64 total = readMemInfoValueKB(QStringLiteral("MemTotal"));
    if (total <= 0)
        return QStringLiteral("Unknown total memory");

    return formatBytes(total * 1024);
}

QString AboutInfo::memoryAvailable() const
{
    const qint64 available = readMemInfoValueKB(QStringLiteral("MemAvailable"));
    if (available <= 0)
        return QStringLiteral("Unknown available memory");

    return formatBytes(available * 1024);
}

void AboutInfo::refreshStorageDevices()
{
    const QVariantList devices = storageDeviceList();
    if (devices == m_storageDevices)
        return;

    m_storageDevices = devices;
    Q_EMIT storageDevicesChanged();
}

QVariantList AboutInfo::storageDevices() const
{
    return m_storageDevices;
}
