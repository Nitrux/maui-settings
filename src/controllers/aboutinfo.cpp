#include "aboutinfo.h"

#include <algorithm>

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QStorageInfo>
#include <QStringList>
#include <QSysInfo>
#include <QVariantMap>
#include <QList>

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

    const QString prefix = QStringLiteral("model name");
    while (!file.atEnd())
    {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (!line.startsWith(prefix))
            continue;

        const QString value = line.section(QLatin1Char(':'), 1).trimmed();
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

bool isInternalFileSystem(const QByteArray &fileSystemType)
{
    static const QList<QByteArray> blacklisted = {
        QByteArrayLiteral("autofs"),
        QByteArrayLiteral("cgroup"),
        QByteArrayLiteral("cgroup2"),
        QByteArrayLiteral("configfs"),
        QByteArrayLiteral("debugfs"),
        QByteArrayLiteral("devpts"),
        QByteArrayLiteral("devtmpfs"),
        QByteArrayLiteral("fuse.portal"),
        QByteArrayLiteral("hugetlbfs"),
        QByteArrayLiteral("mqueue"),
        QByteArrayLiteral("nsfs"),
        QByteArrayLiteral("overlay"),
        QByteArrayLiteral("proc"),
        QByteArrayLiteral("pstore"),
        QByteArrayLiteral("ramfs"),
        QByteArrayLiteral("rpc_pipefs"),
        QByteArrayLiteral("securityfs"),
        QByteArrayLiteral("selinuxfs"),
        QByteArrayLiteral("squashfs"),
        QByteArrayLiteral("sysfs"),
        QByteArrayLiteral("tmpfs"),
        QByteArrayLiteral("tracefs"),
    };

    const QByteArray normalized = fileSystemType.trimmed().toLower();
    if (normalized.isEmpty())
        return true;

    return !blacklisted.contains(normalized);
}

bool isInternalStorage(const QStorageInfo &storage)
{
    if (!storage.isValid() || !storage.isReady())
        return false;

    const QString mountPoint = storage.rootPath();
    if (mountPoint.isEmpty())
        return false;

    if (!isInternalFileSystem(storage.fileSystemType()))
        return false;

    const QByteArray device = storage.device();
    if (!device.isEmpty() && !device.startsWith("/dev/"))
        return false;

    return true;
}

QList<QStorageInfo> internalStorageVolumes()
{
    QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    std::sort(volumes.begin(), volumes.end(), [](const QStorageInfo &left, const QStorageInfo &right) {
        return left.rootPath().compare(right.rootPath()) < 0;
    });

    QList<QStorageInfo> result;
    result.reserve(volumes.size());

    for (const QStorageInfo &storage : volumes)
    {
        if (isInternalStorage(storage))
            result.append(storage);
    }

    return result;
}

QVariantMap storageToMap(const QStorageInfo &storage)
{
    const qint64 total = storage.bytesTotal();
    const qint64 available = storage.bytesAvailable();
    const qint64 used = (total > 0 && available >= 0) ? qMax<qint64>(0, total - available) : -1;

    QVariantMap data;
    data.insert(QStringLiteral("mountPoint"), storage.rootPath().isEmpty() ? QStringLiteral("/") : storage.rootPath());
    data.insert(QStringLiteral("device"), QString::fromUtf8(storage.device()));
    data.insert(QStringLiteral("fileSystem"), storage.fileSystemType().isEmpty() ? QStringLiteral("Unknown file system")
                                                                                   : QString::fromLatin1(storage.fileSystemType()));
    data.insert(QStringLiteral("used"), used >= 0 ? readableDataSize(used) : QStringLiteral("Unknown used space"));
    data.insert(QStringLiteral("available"), available >= 0 ? readableDataSize(available) : QStringLiteral("Unknown available space"));
    data.insert(QStringLiteral("total"), total >= 0 ? readableDataSize(total) : QStringLiteral("Unknown total space"));
    return data;
}

QVariantMap firstInternalStorageMap()
{
    const QList<QStorageInfo> volumes = internalStorageVolumes();
    if (volumes.isEmpty())
        return {};

    return storageToMap(volumes.constFirst());
}
} // namespace

AboutInfo::AboutInfo(QObject *parent)
    : QObject(parent)
{
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
    const QString path = firstExistingPath({QStringLiteral("/run/host/proc/meminfo"), QStringLiteral("/proc/meminfo")});
    const QString value = path.isEmpty() ? QString() : readMemInfoValueKBFromPath(path, key);
    return value.toLongLong();
}

QString AboutInfo::readCpuModel()
{
    const QString path = firstExistingPath({QStringLiteral("/run/host/proc/cpuinfo"), QStringLiteral("/proc/cpuinfo")});
    return path.isEmpty() ? QString() : readCpuModelFromPath(path);
}

QString AboutInfo::osDistribution() const
{
    const QString releasePath = firstExistingPath({QStringLiteral("/run/host/os-release")});
    const QString distribution = releasePath.isEmpty() ? QString() : readReleaseValue(releasePath, QStringLiteral("NAME"));
    if (!distribution.isEmpty())
        return distribution;

    const QString prettyName = releasePath.isEmpty() ? QString() : readReleaseValue(releasePath, QStringLiteral("PRETTY_NAME"));
    if (!prettyName.isEmpty())
        return prettyName;

    const QString productName = QSysInfo::prettyProductName();
    if (!productName.isEmpty())
        return productName;

    return QStringLiteral("Unknown operating system");
}

QString AboutInfo::osVersion() const
{
    const QString releasePath = firstExistingPath({QStringLiteral("/run/host/os-release")});
    const QString version = releasePath.isEmpty() ? QString() : readReleaseValue(releasePath, QStringLiteral("VERSION"));
    if (!version.isEmpty())
        return version;

    const QString versionId = releasePath.isEmpty() ? QString() : readReleaseValue(releasePath, QStringLiteral("VERSION_ID"));
    if (!versionId.isEmpty())
        return versionId;

    const QString productVersion = QSysInfo::productVersion();
    if (!productVersion.isEmpty())
        return productVersion;

    return QStringLiteral("Unknown version");
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
    if (!model.isEmpty())
        return model;

    const QString architecture = QSysInfo::currentCpuArchitecture();
    if (!architecture.isEmpty())
        return architecture;

    return QStringLiteral("Unknown CPU");
}

QString AboutInfo::cpuArchitecture() const
{
    const QString architecture = QSysInfo::currentCpuArchitecture();
    return architecture.isEmpty() ? QStringLiteral("Unknown architecture") : architecture;
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

QVariantList AboutInfo::storageVolumes() const
{
    QVariantList volumes;
    const QList<QStorageInfo> internalVolumes = internalStorageVolumes();
    volumes.reserve(internalVolumes.size());

    for (const QStorageInfo &storage : internalVolumes)
        volumes.append(storageToMap(storage));

    return volumes;
}

QString AboutInfo::storageMountPoint() const
{
    const QVariantMap data = firstInternalStorageMap();
    return data.value(QStringLiteral("mountPoint")).toString();
}

QString AboutInfo::storageFileSystem() const
{
    const QVariantMap data = firstInternalStorageMap();
    return data.value(QStringLiteral("fileSystem")).toString();
}

QString AboutInfo::storageUsed() const
{
    const QVariantMap data = firstInternalStorageMap();
    return data.value(QStringLiteral("used")).toString();
}

QString AboutInfo::storageAvailable() const
{
    const QVariantMap data = firstInternalStorageMap();
    return data.value(QStringLiteral("available")).toString();
}

QString AboutInfo::storageTotal() const
{
    const QVariantMap data = firstInternalStorageMap();
    return data.value(QStringLiteral("total")).toString();
}
