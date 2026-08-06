#include "backgroundinfo.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
#include <QtGlobal>

namespace
{
QString assignmentValue(const QString &line, const QString &key)
{
    const QRegularExpression expression(QStringLiteral(R"(^%1\s*=\s*(.*)$)").arg(QRegularExpression::escape(key)));
    const QRegularExpressionMatch match = expression.match(line.trimmed());
    if (!match.hasMatch())
        return {};

    return match.captured(1).trimmed();
}

QString formatBool(bool value)
{
    return value ? QStringLiteral("true") : QStringLiteral("false");
}

void restartHyprpaper()
{
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("hyprpaper"));
    if (executable.isEmpty())
        return;

    QProcess::execute(QStringLiteral("pkill"), {QStringLiteral("-TERM"), QStringLiteral("-x"), QStringLiteral("hyprpaper")});
    QProcess::startDetached(executable, {});
}
} // namespace

QString BackgroundInfo::homeConfigPath()
{
    return QDir::homePath() + QStringLiteral("/.config/hypr/hyprpaper.conf");
}

QString BackgroundInfo::normalizePath(const QString &value)
{
    if (value.isEmpty())
        return {};

    QString path = value.trimmed();
    if (path.startsWith(QStringLiteral("file:")))
        path = QUrl(path).toLocalFile();

    QFileInfo info(path);
    if (info.exists())
    {
        if (info.isDir())
            return info.absoluteFilePath();

        const QString canonical = info.canonicalFilePath();
        return canonical.isEmpty() ? info.absoluteFilePath() : canonical;
    }

    return info.absoluteFilePath();
}

QString BackgroundInfo::unquoteValue(const QString &value)
{
    QString result = value.trimmed();
    if (result.size() >= 2 && result.startsWith(QLatin1Char('"')) && result.endsWith(QLatin1Char('"')))
    {
        result = result.mid(1, result.size() - 2);
        result.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
        result.replace(QStringLiteral("\\\""), QStringLiteral("\""));
    }

    return result;
}

bool BackgroundInfo::parseBool(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    return normalized == QStringLiteral("1") || normalized == QStringLiteral("true") || normalized == QStringLiteral("yes") || normalized == QStringLiteral("on");
}

BackgroundInfo::BackgroundInfo(QObject *parent)
    : QObject(parent)
    , m_configPath(homeConfigPath())
{
    load();
}

QString BackgroundInfo::configPath() const
{
    return m_configPath;
}

QString BackgroundInfo::wallpaperDirectory() const
{
    if (m_wallpaperPath.isEmpty())
        return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    const QFileInfo info(m_wallpaperPath);
    return info.isDir() ? info.absoluteFilePath() : info.absolutePath();
}

QString BackgroundInfo::wallpaperMonitor() const
{
    return m_wallpaperMonitor;
}

QString BackgroundInfo::wallpaperPath() const
{
    return m_wallpaperPath;
}

QString BackgroundInfo::wallpaperFitMode() const
{
    return m_wallpaperFitMode;
}

int BackgroundInfo::wallpaperTimeout() const
{
    return m_wallpaperTimeout;
}

QString BackgroundInfo::wallpaperOrder() const
{
    return m_wallpaperOrder;
}

bool BackgroundInfo::wallpaperRecursive() const
{
    return m_wallpaperRecursive;
}

bool BackgroundInfo::splashEnabled() const
{
    return m_splashEnabled;
}

int BackgroundInfo::splashOffset() const
{
    return m_splashOffset;
}

double BackgroundInfo::splashOpacity() const
{
    return m_splashOpacity;
}

bool BackgroundInfo::ipcEnabled() const
{
    return m_ipcEnabled;
}

void BackgroundInfo::setChanged()
{
    Q_EMIT settingsChanged();
}

void BackgroundInfo::setWallpaperMonitor(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_wallpaperMonitor == normalized)
        return;

    m_wallpaperMonitor = normalized;
    setChanged();
}

void BackgroundInfo::setWallpaperPath(const QString &value)
{
    const QString normalized = normalizePath(value);
    if (m_wallpaperPath == normalized)
        return;

    m_wallpaperPath = normalized;
    setChanged();
}

void BackgroundInfo::setWallpaperFitMode(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    const QString mode = normalized.isEmpty() ? QStringLiteral("cover") : normalized;
    if (m_wallpaperFitMode == mode)
        return;

    m_wallpaperFitMode = mode;
    setChanged();
}

void BackgroundInfo::setWallpaperTimeout(int value)
{
    value = qMax(0, value);
    if (m_wallpaperTimeout == value)
        return;

    m_wallpaperTimeout = value;
    setChanged();
}

void BackgroundInfo::setWallpaperOrder(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    const QString order = normalized.isEmpty() ? QStringLiteral("default") : normalized;
    if (m_wallpaperOrder == order)
        return;

    m_wallpaperOrder = order;
    setChanged();
}

void BackgroundInfo::setWallpaperRecursive(bool value)
{
    if (m_wallpaperRecursive == value)
        return;

    m_wallpaperRecursive = value;
    setChanged();
}

void BackgroundInfo::setSplashEnabled(bool value)
{
    if (m_splashEnabled == value)
        return;

    m_splashEnabled = value;
    setChanged();
}

void BackgroundInfo::setSplashOffset(int value)
{
    if (m_splashOffset == value)
        return;

    m_splashOffset = value;
    setChanged();
}

void BackgroundInfo::setSplashOpacity(double value)
{
    value = qBound(0.0, value, 1.0);
    if (qFuzzyCompare(m_splashOpacity, value))
        return;

    m_splashOpacity = value;
    setChanged();
}

void BackgroundInfo::setIpcEnabled(bool value)
{
    if (m_ipcEnabled == value)
        return;

    m_ipcEnabled = value;
    setChanged();
}

void BackgroundInfo::reload()
{
    load();
}

void BackgroundInfo::load()
{
    m_wallpaperMonitor.clear();
    m_wallpaperPath.clear();
    m_wallpaperFitMode = QStringLiteral("cover");
    m_wallpaperTimeout = 0;
    m_wallpaperOrder = QStringLiteral("default");
    m_wallpaperRecursive = false;
    m_splashEnabled = false;
    m_splashOffset = 20;
    m_splashOpacity = 0.8;
    m_ipcEnabled = true;

    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setChanged();
        return;
    }

    bool inWallpaperBlock = false;
    while (!file.atEnd())
    {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;

        if (line.startsWith(QStringLiteral("wallpaper")) && line.contains(QLatin1Char('{')))
        {
            inWallpaperBlock = true;
            continue;
        }

        if (inWallpaperBlock && line.startsWith(QLatin1Char('}')))
        {
            inWallpaperBlock = false;
            continue;
        }

        if (inWallpaperBlock)
        {
            const QString monitorValue = assignmentValue(line, QStringLiteral("monitor"));
            const QString pathValue = assignmentValue(line, QStringLiteral("path"));
            const QString fitModeValue = assignmentValue(line, QStringLiteral("fit_mode"));
            const QString timeoutValue = assignmentValue(line, QStringLiteral("timeout"));
            const QString orderValue = assignmentValue(line, QStringLiteral("order"));
            const QString recursiveValue = assignmentValue(line, QStringLiteral("recursive"));

            if (!monitorValue.isEmpty() || line.startsWith(QStringLiteral("monitor")))
                m_wallpaperMonitor = unquoteValue(monitorValue);
            if (!pathValue.isEmpty() || line.startsWith(QStringLiteral("path")))
                m_wallpaperPath = normalizePath(unquoteValue(pathValue));
            if (!fitModeValue.isEmpty() || line.startsWith(QStringLiteral("fit_mode")))
                m_wallpaperFitMode = unquoteValue(fitModeValue).toLower();
            if (!timeoutValue.isEmpty() || line.startsWith(QStringLiteral("timeout")))
                m_wallpaperTimeout = timeoutValue.toInt();
            if (!orderValue.isEmpty() || line.startsWith(QStringLiteral("order")))
                m_wallpaperOrder = unquoteValue(orderValue).toLower();
            if (!recursiveValue.isEmpty() || line.startsWith(QStringLiteral("recursive")))
                m_wallpaperRecursive = parseBool(recursiveValue);
            continue;
        }

        const QString splashValue = assignmentValue(line, QStringLiteral("splash"));
        const QString splashOffsetValue = assignmentValue(line, QStringLiteral("splash_offset"));
        const QString splashOpacityValue = assignmentValue(line, QStringLiteral("splash_opacity"));
        const QString ipcValue = assignmentValue(line, QStringLiteral("ipc"));

        if (!splashValue.isEmpty() || line.startsWith(QStringLiteral("splash")))
            m_splashEnabled = parseBool(splashValue);
        if (!splashOffsetValue.isEmpty() || line.startsWith(QStringLiteral("splash_offset")))
            m_splashOffset = splashOffsetValue.toInt();
        if (!splashOpacityValue.isEmpty() || line.startsWith(QStringLiteral("splash_opacity")))
            m_splashOpacity = splashOpacityValue.toDouble();
        if (!ipcValue.isEmpty() || line.startsWith(QStringLiteral("ipc")))
            m_ipcEnabled = parseBool(ipcValue);
    }

    setChanged();
}

bool BackgroundInfo::save()
{
    if (m_wallpaperPath.isEmpty())
        return false;

    const QFileInfo fileInfo(m_configPath);
    QDir().mkpath(fileInfo.absolutePath());

    QSaveFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << "wallpaper {\n";
    out << "    monitor = " << m_wallpaperMonitor << "\n";
    out << "    path = " << m_wallpaperPath << "\n";
    out << "    fit_mode = " << m_wallpaperFitMode << "\n";
    out << "    timeout = " << m_wallpaperTimeout << "\n";
    out << "    order = " << m_wallpaperOrder << "\n";
    out << "    recursive = " << (m_wallpaperRecursive ? 1 : 0) << "\n";
    out << "}\n\n";
    out << "splash = " << formatBool(m_splashEnabled) << "\n";
    if (m_splashEnabled)
    {
        out << "splash_offset = " << m_splashOffset << "\n";
        out << "splash_opacity = " << QString::number(m_splashOpacity, 'g', 6) << "\n";
    }
    out << "ipc = " << formatBool(m_ipcEnabled) << "\n";

    if (!file.commit())
        return false;

    restartHyprpaper();

    return true;
}
