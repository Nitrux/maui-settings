#include "aboutinfo.h"

#include <algorithm>

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QLocale>
#include <QList>
#include <QProcess>
#include <QStorageInfo>
#include <QStringList>
#include <QSysInfo>
#include <QVariantMap>
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
    qWarning().noquote() << "AboutInfo: trying to read CPU model from" << path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning().noquote() << "AboutInfo: failed to open" << path << "for CPU model:" << file.errorString();
        return {};
    }

    while (!file.atEnd())
    {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        const QString lowerLine = line.toLower();
        if (!lowerLine.contains(QStringLiteral("model name")))
            continue;

        const int separatorIndex = line.indexOf(QLatin1String(":"));
        if (separatorIndex < 0)
        {
            qWarning().noquote() << "AboutInfo: matched model-name line without colon:" << line;
            continue;
        }

        const QString value = line.mid(separatorIndex + 1).trimmed();
        if (!value.isEmpty())
        {
            qWarning().noquote() << "AboutInfo: parsed CPU model:" << value;
            return value;
        }

        qWarning().noquote() << "AboutInfo: matched model-name line but value was empty:" << line;
    }

    qWarning().noquote() << "AboutInfo: no CPU model found in" << path;
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

QString storageLabelPath(const QString &label)
{
    const QString path = QStringLiteral("/dev/disk/by-label/") + label;
    QFileInfo info(path);
    if (!info.exists())
        return {};

    QString target = info.symLinkTarget();
    if (target.isEmpty())
        target = info.canonicalFilePath();
    if (target.isEmpty())
        target = path;

    return normalizeDevicePath(target);
}

QStorageInfo storageForLabel(const QString &label)
{
    const QString target = storageLabelPath(label);
    if (target.isEmpty())
        return {};

    const QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo &storage : volumes)
    {
        if (normalizeDevicePath(QString::fromUtf8(storage.device())) == target)
            return storage;
    }

    return {};
}

QVariantMap storageToMap(const QString &label)
{
    const QStorageInfo storage = storageForLabel(label);
    const bool valid = storage.isValid() && storage.isReady();

    const qint64 total = valid ? storage.bytesTotal() : -1;
    const qint64 available = valid ? storage.bytesAvailable() : -1;
    const qint64 used = (total > 0 && available >= 0) ? qMax<qint64>(0, total - available) : -1;

    QVariantMap data;
    data.insert(QStringLiteral("label"), label);
    data.insert(QStringLiteral("mountPoint"), valid && !storage.rootPath().isEmpty() ? storage.rootPath() : QStringLiteral("Unavailable"));
    data.insert(QStringLiteral("device"), valid && !storage.device().isEmpty() ? QString::fromUtf8(storage.device()) : QStringLiteral("Unavailable"));
    data.insert(QStringLiteral("fileSystem"), valid && !storage.fileSystemType().isEmpty() ? QString::fromLatin1(storage.fileSystemType()) : QStringLiteral("Unknown file system"));
    data.insert(QStringLiteral("used"), used >= 0 ? readableDataSize(used) : QStringLiteral("Unknown used space"));
    data.insert(QStringLiteral("available"), available >= 0 ? readableDataSize(available) : QStringLiteral("Unknown available space"));
    data.insert(QStringLiteral("total"), total >= 0 ? readableDataSize(total) : QStringLiteral("Unknown total space"));
    return data;
}

QVariantMap firstInternalStorageMap()
{
    return storageToMap(QStringLiteral("NX_ROOT"));
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
    qWarning().noquote() << "AboutInfo: reading CPU model with grep from /proc/cpuinfo";

    QProcess process;
    process.start(QStringLiteral("grep"), {QStringLiteral("-m"), QStringLiteral("1"), QStringLiteral("model name"), QStringLiteral("/proc/cpuinfo")});
    if (process.waitForFinished(3000) && process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)
    {
        const QString output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        const int separatorIndex = output.indexOf(QLatin1String(":"));
        if (separatorIndex >= 0)
        {
            const QString model = output.mid(separatorIndex + 1).trimmed();
            if (!model.isEmpty())
            {
                qWarning().noquote() << "AboutInfo: grep returned CPU model:" << model;
                return model;
            }
        }

        qWarning().noquote() << "AboutInfo: grep returned output but it could not be parsed:" << output;
    }
    else
    {
        qWarning().noquote() << "AboutInfo: grep command failed for CPU model:" << process.errorString();
    }

    const QString path = firstExistingPath({QStringLiteral("/proc/cpuinfo")});
    const QString fallbackModel = path.isEmpty() ? QString() : readCpuModelFromPath(path);
    if (!fallbackModel.isEmpty())
        qWarning().noquote() << "AboutInfo: fallback file parser returned CPU model:" << fallbackModel;

    return fallbackModel;
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
    static const QStringList labels = {
        QStringLiteral("NX_ROOT"),
        QStringLiteral("NX_HOME"),
        QStringLiteral("NX_VAR_LIB"),
    };

    QVariantList volumes;
    volumes.reserve(labels.size());

    for (const QString &label : labels)
        volumes.append(storageToMap(label));

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
