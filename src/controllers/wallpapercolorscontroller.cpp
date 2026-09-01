#include "wallpapercolorscontroller.h"

#include "backgroundinfo.h"
#include "kdeglobalsinfo.h"

#include <MauiKit4/Core/colorutils.h>
#include <MauiMan4/thememanager.h>

#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QImage>
#include <QImageReader>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QStringConverter>
#include <QStringList>
#include <QTextStream>
#include <QTimer>

namespace
{
constexpr auto generatedSchemeName = "Maui Wallpaper";
constexpr auto generatedSchemeFileName = "Maui Wallpaper.colors";

QString colorValue(const QColor &color)
{
    return QStringLiteral("%1,%2,%3").arg(color.red()).arg(color.green()).arg(color.blue());
}

void writeColor(QTextStream &out, const QString &key, const QColor &color)
{
    out << key << QLatin1Char(char(61)) << colorValue(color) << QLatin1Char(char(10));
}

void writeColorGroup(QTextStream &out,
                     const QString &group,
                     const QColor &background,
                     const QColor &alternate,
                     const QColor &focus,
                     const QColor &hover,
                     const QColor &active,
                     const QColor &inactive,
                     const QColor &link,
                     const QColor &negative,
                     const QColor &neutral,
                     const QColor &normal,
                     const QColor &positive,
                     const QColor &visited)
{
    out << QLatin1Char(char(91)) << group << QLatin1String("]\n");
    writeColor(out, QStringLiteral("BackgroundAlternate"), alternate);
    writeColor(out, QStringLiteral("BackgroundNormal"), background);
    writeColor(out, QStringLiteral("DecorationFocus"), focus);
    writeColor(out, QStringLiteral("DecorationHover"), hover);
    writeColor(out, QStringLiteral("ForegroundActive"), active);
    writeColor(out, QStringLiteral("ForegroundInactive"), inactive);
    writeColor(out, QStringLiteral("ForegroundLink"), link);
    writeColor(out, QStringLiteral("ForegroundNegative"), negative);
    writeColor(out, QStringLiteral("ForegroundNeutral"), neutral);
    writeColor(out, QStringLiteral("ForegroundNormal"), normal);
    writeColor(out, QStringLiteral("ForegroundPositive"), positive);
    writeColor(out, QStringLiteral("ForegroundVisited"), visited);
    out << QLatin1Char(char(10));
}
}

WallpaperColorsController::WallpaperColorsController(MauiMan::ThemeManager *theme,
                                                       BackgroundInfo *background,
                                                       KdeGlobalsInfo *kde,
                                                       QObject *parent)
    : QObject(parent)
    , m_theme(theme)
    , m_background(background)
    , m_kde(kde)
{
    m_sourceWatcher = new QFileSystemWatcher(this);
    m_sourceSyncTimer = new QTimer(this);
    m_sourceSyncTimer->setSingleShot(true);
    m_sourceSyncTimer->setInterval(100);
    connect(m_sourceWatcher, &QFileSystemWatcher::fileChanged, this, [this]() {
        m_sourceSyncTimer->start();
    });
    connect(m_sourceWatcher, &QFileSystemWatcher::directoryChanged, this, [this]() {
        m_sourceSyncTimer->start();
    });
    connect(m_sourceSyncTimer, &QTimer::timeout, this, &WallpaperColorsController::refreshWallpaperSource);
    QSettings settings;
    settings.beginGroup(QStringLiteral("WallpaperColors"));
    m_kdeSynchronizationEnabled = settings.value(QStringLiteral("SynchronizeKde"), false).toBool();
    m_previousKdeScheme = settings.value(QStringLiteral("PreviousKdeScheme")).toString();
    m_hasPreviousKdeScheme = settings.value(QStringLiteral("PreviousKdeSchemeSet"), settings.contains(QStringLiteral("PreviousKdeScheme"))).toBool();
    settings.endGroup();

    connect(m_background, &BackgroundInfo::wallpaperSourceChanged, this, &WallpaperColorsController::publishWallpaperSource);
    connect(m_theme, &MauiMan::ThemeManager::adaptiveColorSchemeSourceChanged, this, &WallpaperColorsController::onThemeSourceChanged);

    publishWallpaperSource(m_background->wallpaperPath());
}

bool WallpaperColorsController::kdeSynchronizationEnabled() const
{
    return m_kdeSynchronizationEnabled;
}

void WallpaperColorsController::setKdeSynchronizationEnabled(bool enabled)
{
    if (m_kdeSynchronizationEnabled == enabled)
        return;

    if (enabled)
    {
        if (m_kde->colorScheme() != QString::fromLatin1(generatedSchemeName) && !m_hasPreviousKdeScheme)
        {
            m_previousKdeScheme = m_kde->colorScheme();
            m_hasPreviousKdeScheme = true;
        }
        m_kdeSynchronizationEnabled = true;
        persistSettings();
        synchronizeKde(m_currentSource);
    }
    else
    {
        m_kdeSynchronizationEnabled = false;
        restorePreviousScheme();
        persistSettings();
    }

    Q_EMIT kdeSynchronizationEnabledChanged();
}

void WallpaperColorsController::synchronize()
{
    synchronizeKde(m_currentSource);
}

QString WallpaperColorsController::canonicalImagePath(const QString &path)
{
    if (path.trimmed().isEmpty())
        return {};

    const QFileInfo info(path);
    if (!info.isFile() || !info.isReadable())
        return {};

    const QString canonical = info.canonicalFilePath();
    const QString imagePath = canonical.isEmpty() ? info.absoluteFilePath() : canonical;
    QImageReader reader(imagePath);
    if (!reader.canRead())
        return {};

    return imagePath;
}

void WallpaperColorsController::publishWallpaperSource(const QString &path)
{
    m_watchedSourcePath = path.trimmed();
    QString source = canonicalImagePath(m_watchedSourcePath);
    if (m_background->wallpaperTimeout() > 0
        || m_background->wallpaperRecursive()
        || m_background->wallpaperOrder() != QStringLiteral("default"))
    {
        source.clear();
        m_watchedSourcePath.clear();
    }

    m_currentSource = source;
    watchSourceFile(m_watchedSourcePath);
    if (m_theme->adaptiveColorSchemeSource() != source)
        m_theme->setAdaptiveColorSchemeSource(source);
    synchronizeKde(source);
}

void WallpaperColorsController::refreshWallpaperSource()
{
    if (m_watchedSourcePath.isEmpty())
        return;

    publishWallpaperSource(m_watchedSourcePath);
}

void WallpaperColorsController::watchSourceFile(const QString &path)
{
    clearSourceWatcher();
    if (path.trimmed().isEmpty())
        return;

    const QFileInfo info(path);
    if (info.isFile())
        m_sourceWatcher->addPath(info.absoluteFilePath());

    const QString directory = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
    if (QDir(directory).exists())
        m_sourceWatcher->addPath(directory);
}

void WallpaperColorsController::clearSourceWatcher()
{
    if (!m_sourceWatcher)
        return;

    if (!m_sourceWatcher->files().isEmpty())
        m_sourceWatcher->removePaths(m_sourceWatcher->files());
    if (!m_sourceWatcher->directories().isEmpty())
        m_sourceWatcher->removePaths(m_sourceWatcher->directories());
}

void WallpaperColorsController::onThemeSourceChanged(const QString &source)
{
    const QString normalized = canonicalImagePath(source);
    if (!source.trimmed().isEmpty() && normalized.isEmpty())
    {
        m_watchedSourcePath.clear();
        clearSourceWatcher();
        if (m_theme->adaptiveColorSchemeSource() == source)
            m_theme->setAdaptiveColorSchemeSource(QString());
        return;
    }

    if (m_currentSource == normalized)
        return;

    m_watchedSourcePath = source.trimmed();
    m_currentSource = normalized;
    watchSourceFile(m_watchedSourcePath);
    synchronizeKde(m_currentSource);
}

void WallpaperColorsController::synchronizeKde(const QString &source)
{
    if (m_kdeSynchronizationEnabled == false)
        return;

    if (source.isEmpty())
    {
        restorePreviousScheme();
        return;
    }

    if (m_kde->colorScheme() != QString::fromLatin1(generatedSchemeName) && !m_hasPreviousKdeScheme)
    {
        m_previousKdeScheme = m_kde->colorScheme();
        m_hasPreviousKdeScheme = true;
        persistSettings();
    }

    if (writeGeneratedScheme(source))
        m_kde->applyColorSchemeFile(generatedSchemePath(), QString::fromLatin1(generatedSchemeName));
}

bool WallpaperColorsController::writeGeneratedScheme(const QString &source)
{
    const MauiKit::AdaptivePalette palette = MauiKit::AdaptivePalette::fromImage(QImage(source));
    if (palette.valid == false)
        return false;

    const QString path = generatedSchemePath();
    if (QDir().mkpath(QFileInfo(path).absolutePath()) == false)
        return false;

    QSaveFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    const auto writeGroupForPalette = [&out, &palette](const QString &group,
                                             const QColor &background,
                                             const QColor &alternate,
                                             const QColor &focus,
                                             const QColor &hover,
                                             const QColor &normal) {
        writeColorGroup(out, group, background, alternate, focus, hover,
                        palette.activeTextColor, palette.disabledTextColor,
                        palette.linkColor, palette.negativeTextColor,
                        palette.neutralTextColor, normal,
                        palette.positiveTextColor, palette.visitedLinkColor);
    };

    out << QLatin1String("[ColorEffects:Disabled]\nColor=56,56,56\nColorAmount=0\nColorEffect=0\nContrastAmount=0.65\nContrastEffect=1\nIntensityAmount=0.1\nIntensityEffect=2\n\n");
    out << QLatin1String("[ColorEffects:Inactive]\nChangeSelectionColor=true\nColor=112,111,110\nColorAmount=0.025\nColorEffect=2\nContrastAmount=0.1\nContrastEffect=2\nEnable=false\nIntensityAmount=0\nIntensityEffect=0\n\n");

    writeGroupForPalette(QStringLiteral("Colors:Window"), palette.backgroundColor,
                         palette.alternateBackgroundColor, palette.focusColor,
                         palette.hoverColor, palette.textColor);
    writeGroupForPalette(QStringLiteral("Colors:View"), palette.viewBackgroundColor,
                         palette.viewAlternateBackgroundColor, palette.viewFocusColor,
                         palette.viewHoverColor, palette.viewTextColor);
    writeGroupForPalette(QStringLiteral("Colors:Button"), palette.buttonBackgroundColor,
                         palette.buttonAlternateBackgroundColor, palette.buttonFocusColor,
                         palette.buttonHoverColor, palette.buttonTextColor);
    writeGroupForPalette(QStringLiteral("Colors:Selection"), palette.selectionBackgroundColor,
                         palette.selectionAlternateBackgroundColor, palette.selectionFocusColor,
                         palette.selectionHoverColor, palette.selectionTextColor);
    writeGroupForPalette(QStringLiteral("Colors:Tooltip"), palette.tooltipBackgroundColor,
                         palette.tooltipAlternateBackgroundColor, palette.tooltipFocusColor,
                         palette.tooltipHoverColor, palette.tooltipTextColor);
    writeGroupForPalette(QStringLiteral("Colors:Complementary"), palette.complementaryBackgroundColor,
                         palette.complementaryAlternateBackgroundColor, palette.complementaryFocusColor,
                         palette.complementaryHoverColor, palette.complementaryTextColor);
    writeGroupForPalette(QStringLiteral("Colors:Header"), palette.headerBackgroundColor,
                         palette.headerAlternateBackgroundColor, palette.headerFocusColor,
                         palette.headerHoverColor, palette.headerTextColor);

    out << QLatin1String("[General]\nColorScheme=Maui Wallpaper\nName=Maui Wallpaper\nshadeSortColumn=true\n\n[KDE]\ncontrast=4\n\n[WM]\n");
    writeColor(out, QStringLiteral("activeBackground"), palette.backgroundColor);
    writeColor(out, QStringLiteral("activeForeground"), palette.textColor);
    writeColor(out, QStringLiteral("frame"), palette.backgroundColor);
    writeColor(out, QStringLiteral("inactiveBackground"), palette.backgroundColor);
    writeColor(out, QStringLiteral("inactiveForeground"), palette.disabledTextColor);
    writeColor(out, QStringLiteral("inactiveFrame"), palette.backgroundColor);

    return file.commit();
}

void WallpaperColorsController::restorePreviousScheme()
{
    if (m_kde->colorScheme() != QString::fromLatin1(generatedSchemeName) || !m_hasPreviousKdeScheme)
        return;

    m_kde->setColorScheme(m_previousKdeScheme);
    if (m_kde->save())
    {
        m_previousKdeScheme.clear();
        m_hasPreviousKdeScheme = false;
        persistSettings();
    }
}

void WallpaperColorsController::persistSettings() const
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("WallpaperColors"));
    settings.setValue(QStringLiteral("SynchronizeKde"), m_kdeSynchronizationEnabled);
    settings.setValue(QStringLiteral("PreviousKdeScheme"), m_previousKdeScheme);
    settings.setValue(QStringLiteral("PreviousKdeSchemeSet"), m_hasPreviousKdeScheme);
    settings.endGroup();
    settings.sync();
}

QString WallpaperColorsController::generatedSchemePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
        + QStringLiteral("/color-schemes/") + QString::fromLatin1(generatedSchemeFileName);
}
