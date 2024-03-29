#include "aboutdata.h"

#include <QSysInfo>
#include <QProcess>
#include <QStorageInfo>

#include <sys/utsname.h>

AboutData::AboutData(QObject *parent) : QObject(parent)
{
    QProcess process_system;

    QString linuxcpuname = QStringLiteral("cat /proc/cpuinfo | grep 'model name' | uniq");

    process_system.start(linuxcpuname);
    process_system.waitForFinished();

    m_cpuInfo = QString::fromStdString(process_system.readAllStandardOutput().toStdString());
    m_userName = QString::fromStdString(qgetenv("USER").toStdString());

    if (m_userName.isEmpty())
        m_userName = QString::fromStdString(qgetenv("USERNAME").toStdString());

    utsname distroBuf;
    if(uname(&distroBuf))
    {
        m_distroName = QString::fromLocal8Bit(distroBuf.sysname);
        m_distroVersion = QString::fromLocal8Bit(distroBuf.version);
        m_distroWebPage = QString::fromLocal8Bit(distroBuf.domainname);
    }

    for (const QStorageInfo &storage : QStorageInfo::mountedVolumes())
    {
        if (storage.isValid() && storage.isReady())
        {
            if (!storage.isReadOnly())
            {
                QVariantMap device;
                device.insert(QStringLiteral("name"), storage.displayName());
                device.insert(QStringLiteral("root"), storage.rootPath());
                device.insert(QStringLiteral("type"), storage.fileSystemType());
                device.insert(QStringLiteral("bytesAvailable"), storage.bytesAvailable());
                device.insert(QStringLiteral("bytesFree"), storage.bytesFree());
                device.insert(QStringLiteral("bytesTotal"), storage.bytesTotal());
                m_devices << device;

            }
        }
    }
}

QString AboutData::productVersion() const
{
    return QSysInfo::productVersion();
}

QString AboutData::productName() const
{
    return QSysInfo::prettyProductName();
}

QString AboutData::productType() const
{
    return QSysInfo::productType();
}

QString AboutData::machineName() const
{
    return QSysInfo::machineHostName();
}

QString AboutData::kernelType() const
{
    return QSysInfo::kernelType();
}

QString AboutData::kernelVersion() const
{
    return QSysInfo::kernelVersion();
}

QString AboutData::cpuArch() const
{
    return QSysInfo::currentCpuArchitecture();
}

QString AboutData::cpuInfo() const
{
    return m_cpuInfo;
}

QString AboutData::userName() const
{
    return m_userName;
}

QString AboutData::distroWebPage() const
{
    return m_distroWebPage;
}

QString AboutData::distroName() const
{
    return m_distroName;
}

QString AboutData::distroVersion() const
{
    return m_distroVersion;
}

QVariantList AboutData::devices() const
{
    return m_devices;
}
