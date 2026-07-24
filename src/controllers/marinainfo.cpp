#include "marinainfo.h"

#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QtGlobal>

namespace
{
bool restartDetached(const QString &processName, const QString &programName)
{
    const QString shell = QStandardPaths::findExecutable(QStringLiteral("sh"));
    const QString pkill = QStandardPaths::findExecutable(QStringLiteral("pkill"));
    const QString program = QStandardPaths::findExecutable(programName);
    if (shell.isEmpty() || pkill.isEmpty() || program.isEmpty())
        return false;

    // Keep the delay in the detached helper so closing the settings app cannot cancel the relaunch.
    const QString script = QStringLiteral("\"$1\" -x -- \"$2\" >/dev/null 2>&1 || true; sleep 0.2; exec \"$3\" >/dev/null 2>&1");
    return QProcess::startDetached(shell, {QStringLiteral("-c"), script, QStringLiteral("workspace-settings-restart"), pkill, processName, program});
}
} // namespace

MarinaInfo::MarinaInfo(QObject *parent)
    : QObject(parent)
    , m_configPath(QDir::homePath() + QStringLiteral("/.config/marina/marina.conf"))
{
    load();
}

QString MarinaInfo::configPath() const { return m_configPath; }
int MarinaInfo::iconSize() const { return m_iconSize; }
int MarinaInfo::edgeMargin() const { return m_edgeMargin; }
QString MarinaInfo::screenPlacement() const { return m_screenPlacement; }
int MarinaInfo::dockWidth() const { return m_dockWidth; }
int MarinaInfo::dockHeight() const { return m_dockHeight; }
bool MarinaInfo::showAboveFullscreen() const { return m_showAboveFullscreen; }
bool MarinaInfo::autoHide() const { return m_autoHide; }
int MarinaInfo::autoHideDelay() const { return m_autoHideDelay; }

void MarinaInfo::setChanged() { Q_EMIT settingsChanged(); }

void MarinaInfo::setIconSize(int value)
{
    value = qBound(32, value, 96);
    if (m_iconSize == value)
        return;

    m_iconSize = value;
    if (m_dockHeight > 0)
        m_dockHeight = qBound(m_iconSize + 8, m_dockHeight, 256);
    setChanged();
}

void MarinaInfo::setEdgeMargin(int value)
{
    value = qBound(0, value, 48);
    if (m_edgeMargin == value)
        return;
    m_edgeMargin = value;
    setChanged();
}

void MarinaInfo::setScreenPlacement(const QString &value)
{
    const QString normalized = value.trimmed().toLower() == QLatin1String("active")
        ? QStringLiteral("active")
        : QStringLiteral("all");
    if (m_screenPlacement == normalized)
        return;
    m_screenPlacement = normalized;
    setChanged();
}

void MarinaInfo::setDockWidth(int value)
{
    value = value <= 0 ? 0 : qBound(96, value, 4096);
    if (m_dockWidth == value)
        return;
    m_dockWidth = value;
    setChanged();
}

void MarinaInfo::setDockHeight(int value)
{
    value = value <= 0 ? 0 : qBound(m_iconSize + 8, value, 256);
    if (m_dockHeight == value)
        return;
    m_dockHeight = value;
    setChanged();
}

void MarinaInfo::setShowAboveFullscreen(bool value)
{
    if (m_showAboveFullscreen == value)
        return;
    m_showAboveFullscreen = value;
    setChanged();
}

void MarinaInfo::setAutoHide(bool value)
{
    if (m_autoHide == value)
        return;
    m_autoHide = value;
    setChanged();
}

void MarinaInfo::setAutoHideDelay(int value)
{
    value = qBound(0, value, 5000);
    if (m_autoHideDelay == value)
        return;
    m_autoHideDelay = value;
    setChanged();
}

void MarinaInfo::reload()
{
    load();
}

void MarinaInfo::load()
{
    const QSettings settings(m_configPath, QSettings::IniFormat);
    m_iconSize = qBound(32, settings.value(QStringLiteral("Appearance/iconSize"), 48).toInt(), 96);
    m_edgeMargin = qBound(0, settings.value(QStringLiteral("Appearance/edgeMargin"), 8).toInt(), 48);
    m_screenPlacement = settings.value(QStringLiteral("Window/screenPlacement"), QStringLiteral("all")).toString().trimmed().toLower() == QLatin1String("active")
        ? QStringLiteral("active")
        : QStringLiteral("all");

    const int requestedWidth = settings.value(QStringLiteral("Window/width"), 0).toInt();
    const int requestedHeight = settings.value(QStringLiteral("Window/height"), 0).toInt();
    m_dockWidth = requestedWidth <= 0 ? 0 : qBound(96, requestedWidth, 4096);
    m_dockHeight = requestedHeight <= 0 ? 0 : qBound(m_iconSize + 8, requestedHeight, 256);
    m_showAboveFullscreen = settings.value(QStringLiteral("Window/showAboveFullscreen"), false).toBool();
    m_autoHide = settings.value(QStringLiteral("Behavior/autoHide"), false).toBool();
    m_autoHideDelay = qBound(0, settings.value(QStringLiteral("Behavior/autoHideDelay"), 650).toInt(), 5000);
    setChanged();
}

bool MarinaInfo::save()
{
    QDir().mkpath(QDir::homePath() + QStringLiteral("/.config/marina"));
    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("Appearance/iconSize"), m_iconSize);
    settings.setValue(QStringLiteral("Appearance/edgeMargin"), m_edgeMargin);
    settings.setValue(QStringLiteral("Window/screenPlacement"), m_screenPlacement);
    settings.setValue(QStringLiteral("Window/width"), m_dockWidth);
    settings.setValue(QStringLiteral("Window/height"), m_dockHeight);
    settings.setValue(QStringLiteral("Window/showAboveFullscreen"), m_showAboveFullscreen);
    settings.setValue(QStringLiteral("Behavior/autoHide"), m_autoHide);
    settings.setValue(QStringLiteral("Behavior/autoHideDelay"), m_autoHideDelay);
    settings.sync();
    if (settings.status() != QSettings::NoError)
        return false;

    return restartDetached(QStringLiteral("marina"), QStringLiteral("marina"));
}
