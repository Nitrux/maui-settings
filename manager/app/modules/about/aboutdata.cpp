#include "aboutdata.h"
#include <QSysInfo>
#include <QProcess>
#include <sys/utsname.h>

AboutData::AboutData(QObject *parent) : QObject(parent)
{
    QProcess process_system;

    QString linuxcpuname = "cat /proc/cpuinfo | grep 'model name' | uniq";
    process_system.start(linuxcpuname);
    process_system.waitForFinished();
    m_cpuInfo = process_system.readAllStandardOutput();

    m_userName= qgetenv("USER");
    if (m_userName.isEmpty())
        m_userName = qgetenv("USERNAME");

     utsname distroBuf;
     if(uname(&distroBuf))
     {
         m_distroName = distroBuf.sysname;
         m_distroVersion = distroBuf.version;
         m_distroWebPage = distroBuf.domainname;
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
