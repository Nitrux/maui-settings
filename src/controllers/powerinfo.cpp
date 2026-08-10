#include "powerinfo.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

PowerInfo::PowerInfo(QObject *parent)
    : QObject(parent)
    , m_configPath(QDir::homePath() + QStringLiteral("/.config/nx-powerd/nx-powerd.conf"))
{
    reload();
}

QString PowerInfo::configPath() const { return m_configPath; }
bool PowerInfo::enabled() const { return m_enabled; }
int PowerInfo::powerSaverMax() const { return m_powerSaverMax; }
int PowerInfo::balancedMax() const { return m_balancedMax; }
int PowerInfo::performanceMin() const { return m_performanceMin; }
bool PowerInfo::daemonAvailable() const { return m_daemonAvailable; }
bool PowerInfo::daemonRunning() const { return m_daemonRunning; }
bool PowerInfo::configAvailable() const { return m_configAvailable; }

bool PowerInfo::daemonIsRunning() const
{
    const QDir proc(QStringLiteral("/proc"));
    const QStringList entries = proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &entry : entries) {
        bool ok = false;
        entry.toLongLong(&ok);
        if (!ok)
            continue;

        QFile comm(proc.filePath(entry + QStringLiteral("/comm")));
        if (!comm.open(QIODevice::ReadOnly))
            continue;

        if (QString::fromUtf8(comm.readAll()).trimmed() == QStringLiteral("nx-powerd"))
            return true;
    }

    return false;
}

void PowerInfo::setEnabled(bool value)
{
    if (m_enabled == value)
        return;
    m_enabled = value;
    Q_EMIT settingsChanged();
}

void PowerInfo::setPowerSaverMax(int value)
{
    value = qBound(0, value, 98);
    if (m_powerSaverMax == value)
        return;
    m_powerSaverMax = value;
    m_balancedMax = qMax(m_balancedMax, m_powerSaverMax + 1);
    m_performanceMin = qMax(m_performanceMin, m_balancedMax + 1);
    Q_EMIT settingsChanged();
}

void PowerInfo::setBalancedMax(int value)
{
    value = qBound(m_powerSaverMax + 1, value, 99);
    if (m_balancedMax == value)
        return;
    m_balancedMax = value;
    m_performanceMin = qMax(m_performanceMin, m_balancedMax + 1);
    Q_EMIT settingsChanged();
}

void PowerInfo::setPerformanceMin(int value)
{
    value = qBound(m_balancedMax + 1, value, 100);
    if (m_performanceMin == value)
        return;
    m_performanceMin = value;
    Q_EMIT settingsChanged();
}

void PowerInfo::reload()
{
    m_daemonAvailable = !QStandardPaths::findExecutable(QStringLiteral("nx-powerd")).isEmpty();
    m_daemonRunning = m_daemonAvailable && daemonIsRunning();

    const QFileInfo configInfo(m_configPath);
    m_configAvailable = configInfo.exists() && configInfo.isFile();

    load();
}

void PowerInfo::load()
{
    const QSettings settings(m_configPath, QSettings::IniFormat);
    m_enabled = settings.value(QStringLiteral("Daemon/enabled"), true).toBool();
    m_powerSaverMax = qBound(0, settings.value(QStringLiteral("Profiles/powerSaverMax"), 20).toInt(), 98);
    m_balancedMax = qBound(m_powerSaverMax + 1, settings.value(QStringLiteral("Profiles/balancedMax"), 59).toInt(), 99);
    m_performanceMin = qBound(m_balancedMax + 1, settings.value(QStringLiteral("Profiles/performanceMin"), 60).toInt(), 100);
    Q_EMIT settingsChanged();
}

bool PowerInfo::save()
{
    QDir().mkpath(QDir::homePath() + QStringLiteral("/.config/nx-powerd"));
    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("Daemon/enabled"), m_enabled);
    settings.setValue(QStringLiteral("Profiles/powerSaverMax"), m_powerSaverMax);
    settings.setValue(QStringLiteral("Profiles/balancedMax"), m_balancedMax);
    settings.setValue(QStringLiteral("Profiles/performanceMin"), m_performanceMin);
    settings.sync();

    return settings.status() == QSettings::NoError;
}