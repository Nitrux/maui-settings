#include "qmllogoutcontroller.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QtGlobal>

QmlLogoutController::QmlLogoutController(QObject *parent)
    : QObject(parent)
    , m_configPath([]() {
        const QString configDirectory = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
        return (configDirectory.isEmpty() ? QDir::homePath() + QStringLiteral("/.config") : configDirectory)
            + QStringLiteral("/qmlogout/qmlogout.conf");
    }())
    , m_available(!QStandardPaths::findExecutable(QStringLiteral("qmlogout")).isEmpty())
{
    load();
}

QString QmlLogoutController::configPath() const { return m_configPath; }
bool QmlLogoutController::available() const { return m_available; }
QString QmlLogoutController::iconMode() const { return m_iconMode; }
double QmlLogoutController::overlayOpacity() const { return m_overlayOpacity; }
QString QmlLogoutController::avatarPath() const { return m_avatarPath; }
bool QmlLogoutController::showUptime() const { return m_showUptime; }
int QmlLogoutController::actionTimeout() const { return m_actionTimeout; }

void QmlLogoutController::setChanged()
{
    Q_EMIT settingsChanged();
}

void QmlLogoutController::setIconMode(const QString &value)
{
    const QString normalized = value.trimmed().toLower() == QLatin1String("nerd")
        ? QStringLiteral("nerd") : QStringLiteral("system");
    if (m_iconMode == normalized)
        return;
    m_iconMode = normalized;
    setChanged();
}

void QmlLogoutController::setOverlayOpacity(double value)
{
    const double normalized = qBound(0.0, value, 1.0);
    if (qFuzzyCompare(m_overlayOpacity, normalized))
        return;
    m_overlayOpacity = normalized;
    setChanged();
}

void QmlLogoutController::setAvatarPath(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_avatarPath == normalized)
        return;
    m_avatarPath = normalized;
    setChanged();
}

void QmlLogoutController::setShowUptime(bool value)
{
    if (m_showUptime == value)
        return;
    m_showUptime = value;
    setChanged();
}

void QmlLogoutController::setActionTimeout(int value)
{
    const int normalized = qBound(1, value, 120);
    if (m_actionTimeout == normalized)
        return;
    m_actionTimeout = normalized;
    setChanged();
}

void QmlLogoutController::reload()
{
    load();
}

void QmlLogoutController::load()
{
    QSettings settings(m_configPath, QSettings::IniFormat);
    m_overlayOpacity = qBound(0.0, settings.value(QStringLiteral("Appearance/OverlayOpacity"), 0.76).toDouble(), 1.0);
    m_iconMode = settings.value(QStringLiteral("Appearance/IconMode"), QStringLiteral("system")).toString().trimmed().toLower() == QLatin1String("nerd")
        ? QStringLiteral("nerd") : QStringLiteral("system");
    m_avatarPath = settings.value(QStringLiteral("Appearance/AvatarImage"), QString()).toString().trimmed();
    m_showUptime = settings.value(QStringLiteral("SystemUptime/ShowUptime"), true).toBool();
    m_actionTimeout = qBound(1, settings.value(QStringLiteral("Session/ActionTimeout"), 30).toInt(), 120);
    setChanged();
}

bool QmlLogoutController::save()
{
    QDir().mkpath(QFileInfo(m_configPath).absolutePath());
    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("Appearance/OverlayOpacity"), m_overlayOpacity);
    settings.setValue(QStringLiteral("Appearance/IconMode"), m_iconMode);
    settings.setValue(QStringLiteral("Appearance/AvatarImage"), m_avatarPath);
    settings.setValue(QStringLiteral("SystemUptime/ShowUptime"), m_showUptime);
    settings.setValue(QStringLiteral("Session/ActionTimeout"), m_actionTimeout);
    settings.sync();
    return settings.status() == QSettings::NoError;
}