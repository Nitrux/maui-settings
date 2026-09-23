#include "wallpapercolorscontroller.h"

#include "backgroundinfo.h"
#include "kdeglobalsinfo.h"
#include "hyprlandinfo.h"

#include <MauiKit4/Core/colorutils.h>
#include <MauiMan4/thememanager.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QGuiApplication>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QImage>
#include <QImageReader>
#include <QProcess>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QSaveFile>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleHints>
#include <QStringConverter>
#include <QStringList>
#include <QTextStream>
#include <QTimer>

namespace
{
constexpr auto generatedSchemeName = "Maui Wallpaper";
constexpr auto generatedSchemeFileName = "Maui Wallpaper.colors";
constexpr auto generatedVicinaeDarkThemeId = "maui-wallpaper-dark";
constexpr auto generatedVicinaeLightThemeId = "maui-wallpaper-light";
enum class MauiStyleType : int
{
    Light = 0,
    Dark,
    Adaptive,
    Auto,
    TrueBlack,
    Inverted
};

QByteArray removeJsonComments(const QByteArray &input)
{
    QByteArray output;
    output.reserve(input.size());

    bool inString = false;
    bool escaped = false;
    bool lineComment = false;
    bool blockComment = false;
    for (qsizetype i = 0; i < input.size(); ++i)
    {
        const char current = input.at(i);
        const char next = i + 1 < input.size() ? input.at(i + 1) : char(0);

        if (lineComment)
        {
            if (current == char(10) || current == char(13))
            {
                lineComment = false;
                output.append(current);
            }
            else
            {
                output.append(char(32));
            }
            continue;
        }

        if (blockComment)
        {
            if (current == char(42) && next == char(47))
            {
                blockComment = false;
                output.append("  ");
                ++i;
            }
            else
            {
                output.append(current == char(10) || current == char(13) ? current : char(32));
            }
            continue;
        }

        if (inString)
        {
            output.append(current);
            if (escaped)
                escaped = false;
            else if (current == char(92))
                escaped = true;
            else if (current == char(34))
                inString = false;
            continue;
        }

        if (current == char(34))
        {
            inString = true;
            output.append(current);
        }
        else if (current == char(47) && next == char(47))
        {
            lineComment = true;
            output.append("  ");
            ++i;
        }
        else if (current == char(47) && next == char(42))
        {
            blockComment = true;
            output.append("  ");
            ++i;
        }
        else
        {
            output.append(current);
        }
    }

    return output;
}

QByteArray removeTrailingJsonCommas(const QByteArray &input)
{
    QByteArray output;
    output.reserve(input.size());
    bool inString = false;
    bool escaped = false;
    for (qsizetype i = 0; i < input.size(); ++i)
    {
        const char current = input.at(i);
        if (inString)
        {
            output.append(current);
            if (escaped)
                escaped = false;
            else if (current == char(92))
                escaped = true;
            else if (current == char(34))
                inString = false;
            continue;
        }

        if (current == char(34))
        {
            inString = true;
            output.append(current);
            continue;
        }

        if (current == char(44))
        {
            qsizetype next = i + 1;
            while (next < input.size() && (input.at(next) == char(32) || input.at(next) == char(9)
                                            || input.at(next) == char(10) || input.at(next) == char(13)))
                ++next;
            if (next < input.size() && (input.at(next) == char(125) || input.at(next) == char(93)))
                continue;
        }

        output.append(current);
    }
    return output;
}

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
                                                       HyprlandInfo *hyprland,
                                                       QObject *parent)
    : QObject(parent)
    , m_theme(theme)
    , m_background(background)
    , m_kde(kde)
    , m_hyprland(hyprland)
{
    m_vicinaeAvailable = !QStandardPaths::findExecutable(QStringLiteral("vicinae")).isEmpty();
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
    const bool legacySynchronizationEnabled = settings.value(QStringLiteral("SynchronizeKde"), false).toBool();
    m_vicinaeSynchronizationEnabled = m_vicinaeAvailable
        && settings.value(QStringLiteral("SynchronizeVicinae"), false).toBool();
    m_kdeSynchronizationEnabled = m_theme->adaptiveColorSchemeEnabled();
    m_previousKdeScheme = settings.value(QStringLiteral("PreviousKdeScheme")).toString();
    m_hasPreviousKdeScheme = settings.value(QStringLiteral("PreviousKdeSchemeSet"), settings.contains(QStringLiteral("PreviousKdeScheme"))).toBool();
    settings.endGroup();

    if (legacySynchronizationEnabled && !m_kdeSynchronizationEnabled)
    {
        m_kdeSynchronizationEnabled = true;
        restorePreviousScheme();
        m_kde->synchronizeGreeter();
        m_kdeSynchronizationEnabled = false;
        persistSettings();
    }

    connect(m_background, &BackgroundInfo::wallpaperSourceChanged, this, &WallpaperColorsController::publishWallpaperSource);
    connect(m_background, &BackgroundInfo::wallpaperSourceSaved, this, &WallpaperColorsController::synchronizeWallpaperSource);
    connect(m_theme, &MauiMan::ThemeManager::adaptiveColorSchemeSourceChanged, this, &WallpaperColorsController::onThemeSourceChanged);

    updateWallpaperSource(m_background->wallpaperPath(), false);
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
    synchronizeKde(m_currentSource, true);
    synchronizeVicinae(m_currentSource);
}

bool WallpaperColorsController::vicinaeAvailable() const
{
    return m_vicinaeAvailable;
}

bool WallpaperColorsController::vicinaeSynchronizationEnabled() const
{
    return m_vicinaeSynchronizationEnabled;
}

void WallpaperColorsController::setVicinaeSynchronizationEnabled(bool enabled)
{
    enabled = enabled && m_vicinaeAvailable;
    if (m_vicinaeSynchronizationEnabled == enabled)
        return;

    m_vicinaeSynchronizationEnabled = enabled;
    persistSettings();
    Q_EMIT vicinaeSynchronizationEnabledChanged();
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
    updateWallpaperSource(path, false);
}

void WallpaperColorsController::synchronizeWallpaperSource(const QString &path)
{
    updateWallpaperSource(path, true);
}

void WallpaperColorsController::updateWallpaperSource(const QString &path, bool synchronizeGreeter)
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
    synchronizeKde(source, synchronizeGreeter);
    synchronizeVicinae(source);
}

void WallpaperColorsController::refreshWallpaperSource()
{
    if (m_watchedSourcePath.isEmpty())
        return;

    synchronizeWallpaperSource(m_watchedSourcePath);
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
    synchronizeKde(m_currentSource, true);
    synchronizeVicinae(m_currentSource);
}

void WallpaperColorsController::synchronizeKde(const QString &source, bool synchronizeGreeter)
{
    if (m_kdeSynchronizationEnabled == false)
        return;

    if (source.isEmpty())
    {
        restorePreviousScheme();
        if (synchronizeGreeter)
            m_kde->synchronizeGreeter();
        return;
    }

    if (m_kde->colorScheme() != QString::fromLatin1(generatedSchemeName) && !m_hasPreviousKdeScheme)
    {
        m_previousKdeScheme = m_kde->colorScheme();
        m_hasPreviousKdeScheme = true;
        persistSettings();
    }

    const QImage image(source);
    const MauiKit::AdaptivePalette palette = MauiKit::AdaptivePalette::fromImage(image);
    if (!palette.valid)
        return;

    if (!writeGeneratedScheme(palette)
        || !m_kde->applyColorSchemeFile(generatedSchemePath(), QString::fromLatin1(generatedSchemeName)))
        return;

    synchronizeHyprlandBorders(palette);
    if (synchronizeGreeter)
        m_kde->synchronizeGreeter();
}

void WallpaperColorsController::synchronizeVicinae(const QString &source)
{
    if (!m_vicinaeAvailable || !m_vicinaeSynchronizationEnabled)
        return;

    bool generatedThemes = false;
    bool useLightTheme = false;
    if (!source.isEmpty())
    {
        const QImage image(source);
        const MauiKit::AdaptivePalette palette = MauiKit::AdaptivePalette::fromImage(image);
        const MauiKit::AdaptivePalette darkPalette =
            MauiKit::AdaptivePalette::fromImage(image, MauiKit::AdaptivePaletteMode::Dark);
        const MauiKit::AdaptivePalette lightPalette =
            MauiKit::AdaptivePalette::fromImage(image, MauiKit::AdaptivePaletteMode::Light);

        if (palette.valid && darkPalette.valid && lightPalette.valid
            && writeGeneratedVicinaeTheme(darkPalette, QString::fromLatin1(generatedVicinaeDarkThemeId),
                                           QStringLiteral("dark"), QStringLiteral("vicinae-dark"))
            && writeGeneratedVicinaeTheme(lightPalette, QString::fromLatin1(generatedVicinaeLightThemeId),
                                          QStringLiteral("light"), QStringLiteral("vicinae-light")))
        {
            generatedThemes = true;
            useLightTheme = useLightVicinaeTheme(palette);
        }
    }

    if (!writeVicinaeSettings(generatedThemes, useLightTheme))
        qWarning() << "Failed to synchronize Vicinae settings";
    else if (generatedThemes)
        refreshVicinaeTheme(useLightTheme ? QString::fromLatin1(generatedVicinaeLightThemeId)
                                          : QString::fromLatin1(generatedVicinaeDarkThemeId));
}

void WallpaperColorsController::refreshVicinaeTheme(const QString &themeId)
{
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("vicinae"));
    if (executable.isEmpty() || themeId.isEmpty())
        return;

    if (QProcess::execute(executable, {QStringLiteral("theme"), QStringLiteral("set"), themeId}) != 0)
        qWarning() << "Failed to refresh Vicinae theme" << themeId;
}

void WallpaperColorsController::synchronizeHyprlandBorders(const MauiKit::AdaptivePalette &palette)
{
    if (!m_hyprland || !m_hyprland->available() || !m_hyprland->borderColorsFollowTheme())
        return;

    if (!palette.valid)
        return;

    const auto rgba = [](const QColor &color, int alpha) {
        return QStringLiteral("rgba(%1%2%3%4)")
            .arg(color.red(), 2, 16, QLatin1Char('0'))
            .arg(color.green(), 2, 16, QLatin1Char('0'))
            .arg(color.blue(), 2, 16, QLatin1Char('0'))
            .arg(alpha, 2, 16, QLatin1Char('0'));
    };

    const QString startColor = rgba(palette.highlightColor, 0xff);
    const QString endColor = rgba(palette.highlightColor.lighter(120), 0xff);
    const QString inactiveColor = rgba(palette.disabledTextColor, 0xaa);
    if (m_hyprland->activeBorderColorStart() == startColor
        && m_hyprland->activeBorderColorEnd() == endColor
        && m_hyprland->inactiveBorderColor() == inactiveColor)
        return;

    m_hyprland->setActiveBorderColorStart(startColor);
    m_hyprland->setActiveBorderColorEnd(endColor);
    m_hyprland->setInactiveBorderColor(inactiveColor);
    m_hyprland->save();
}

bool WallpaperColorsController::writeGeneratedScheme(const MauiKit::AdaptivePalette &palette)
{
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

bool WallpaperColorsController::writeGeneratedVicinaeTheme(const MauiKit::AdaptivePalette &palette,
                                                               const QString &themeId,
                                                               const QString &variant,
                                                               const QString &parentId)
{
    const QString path = generatedVicinaeThemePath(themeId);
    if (QDir().mkpath(QFileInfo(path).absolutePath()) == false)
        return false;

    QSaveFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
        return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    const auto writeColor = [&out](const QString &key, const QColor &color) {
        const auto format = color.alpha() == 0xff ? QColor::HexRgb : QColor::HexArgb;
        out << key << QLatin1String(" = \"") << color.name(format) << QLatin1String("\"\n");
    };
    const auto withOpacity = [](QColor color, qreal opacity) {
        color.setAlphaF(opacity);
        return color;
    };

    out << QLatin1String("[meta]\n");
    out << QLatin1String("name = \"")
        << (variant == QStringLiteral("dark") ? QStringLiteral("Maui Wallpaper Dark")
                                                : QStringLiteral("Maui Wallpaper Light"))
        << QLatin1String("\"\n");
    out << QLatin1String("description = \"Generated from the current wallpaper\"\n");
    out << QLatin1String("variant = \"") << variant << QLatin1String("\"\n");
    out << QLatin1String("inherits = \"") << parentId << QLatin1String("\"\n\n");

    out << QLatin1String("[colors.core]\n");
    writeColor(QStringLiteral("accent"), palette.highlightColor);
    writeColor(QStringLiteral("accent_foreground"), palette.highlightedTextColor);
    writeColor(QStringLiteral("background"), palette.backgroundColor);
    writeColor(QStringLiteral("foreground"), palette.textColor);
    writeColor(QStringLiteral("secondary_background"), palette.viewBackgroundColor);
    writeColor(QStringLiteral("border"), palette.viewAlternateBackgroundColor);

    out << QLatin1String("\n[colors.main_window]\n");
    writeColor(QStringLiteral("border"), palette.alternateBackgroundColor);
    writeColor(QStringLiteral("footer.background"), palette.alternateBackgroundColor);

    out << QLatin1String("\n[colors.settings_window]\n");
    writeColor(QStringLiteral("border"), palette.alternateBackgroundColor);

    out << QLatin1String("\n[colors.accents]\n");
    writeColor(QStringLiteral("blue"), palette.linkColor);
    writeColor(QStringLiteral("green"), palette.positiveBackgroundColor);
    writeColor(QStringLiteral("magenta"), palette.highlightColor);
    writeColor(QStringLiteral("orange"), palette.neutralBackgroundColor);
    writeColor(QStringLiteral("red"), palette.negativeBackgroundColor);
    writeColor(QStringLiteral("yellow"), palette.highlightColor.lighter(115));
    writeColor(QStringLiteral("cyan"), palette.activeTextColor);
    writeColor(QStringLiteral("purple"), palette.visitedLinkColor);

    out << QLatin1String("\n[colors.shortcut]\n");
    writeColor(QStringLiteral("border"), palette.viewAlternateBackgroundColor);

    out << QLatin1String("\n[colors.text]\n");
    writeColor(QStringLiteral("default"), palette.textColor);
    writeColor(QStringLiteral("muted"), palette.disabledTextColor);
    writeColor(QStringLiteral("danger"), palette.negativeTextColor);
    writeColor(QStringLiteral("success"), palette.positiveTextColor);
    writeColor(QStringLiteral("placeholder"), palette.disabledTextColor);

    out << QLatin1String("\n[colors.text.links]\n");
    writeColor(QStringLiteral("default"), palette.linkColor);
    writeColor(QStringLiteral("visited"), palette.visitedLinkColor);

    out << QLatin1String("\n[colors.text.selection]\n");
    writeColor(QStringLiteral("background"), palette.selectionBackgroundColor);
    writeColor(QStringLiteral("foreground"), palette.selectionTextColor);

    out << QLatin1String("\n[colors.input]\n");
    writeColor(QStringLiteral("background"), palette.viewBackgroundColor);
    writeColor(QStringLiteral("border"), palette.viewAlternateBackgroundColor);
    writeColor(QStringLiteral("border_focus"), palette.viewFocusColor);
    writeColor(QStringLiteral("border_error"), palette.negativeBackgroundColor);

    out << QLatin1String("\n[colors.button.primary]\n");
    writeColor(QStringLiteral("background"), palette.buttonBackgroundColor);
    writeColor(QStringLiteral("foreground"), palette.buttonTextColor);
    writeColor(QStringLiteral("hover.background"), palette.buttonHoverColor);
    writeColor(QStringLiteral("hover.foreground"), palette.buttonTextColor);
    writeColor(QStringLiteral("focus.outline"), palette.buttonFocusColor);

    out << QLatin1String("\n[colors.list.item.hover]\n");
    writeColor(QStringLiteral("background"), palette.hoverColor);
    writeColor(QStringLiteral("foreground"), palette.textColor);
    writeColor(QStringLiteral("secondary_foreground"), palette.disabledTextColor);

    out << QLatin1String("\n[colors.list.item.selection]\n");
    writeColor(QStringLiteral("background"), palette.selectionBackgroundColor);
    writeColor(QStringLiteral("foreground"), palette.selectionTextColor);
    writeColor(QStringLiteral("secondary_background"), palette.selectionAlternateBackgroundColor);
    writeColor(QStringLiteral("secondary_foreground"), palette.selectionTextColor);

    out << QLatin1String("\n[colors.grid.item]\n");
    writeColor(QStringLiteral("background"), palette.viewBackgroundColor);
    writeColor(QStringLiteral("hover.outline"), palette.viewFocusColor);
    writeColor(QStringLiteral("selection.outline"), palette.selectionFocusColor);

    out << QLatin1String("\n[colors.scrollbars]\n");
    writeColor(QStringLiteral("background"), withOpacity(palette.textColor, 0.25));
    writeColor(QStringLiteral("secondary_background"), withOpacity(palette.textColor, 0.15));

    out << QLatin1String("\n[colors.tooltip]\n");
    writeColor(QStringLiteral("background"), palette.tooltipBackgroundColor);
    writeColor(QStringLiteral("foreground"), palette.tooltipTextColor);
    writeColor(QStringLiteral("border"), palette.tooltipFocusColor);

    out << QLatin1String("\n[colors.loading]\n");
    writeColor(QStringLiteral("bar"), palette.highlightColor);
    writeColor(QStringLiteral("spinner"), palette.textColor);

    return file.commit();
}

bool WallpaperColorsController::writeVicinaeSettings(bool generatedThemes, bool useLightTheme)
{
    const QString path = vicinaeSettingsPath();
    if (path.isEmpty() || !QDir().mkpath(QFileInfo(path).absolutePath()))
        return false;

    QJsonObject root;
    QFile input(path);
    if (input.exists())
    {
        if (!input.open(QIODevice::ReadOnly))
            return false;

        QJsonParseError error;
        const QByteArray json = removeTrailingJsonCommas(removeJsonComments(input.readAll()));
        const QJsonDocument document = QJsonDocument::fromJson(json, &error);
        if (error.error != QJsonParseError::NoError || !document.isObject())
        {
            qWarning() << "Failed to parse Vicinae settings" << path << error.errorString();
            return false;
        }
        root = document.object();
    }

    const QFont font = m_kde->fontFromString(m_kde->defaultFont());
    QJsonObject fontConfig = root.value(QStringLiteral("font")).toObject();
    QJsonObject normalFont = fontConfig.value(QStringLiteral("normal")).toObject();
    if (!font.family().trimmed().isEmpty())
        normalFont.insert(QStringLiteral("family"), font.family());
    const qreal pointSize = font.pointSizeF();
    if (pointSize > 0)
        normalFont.insert(QStringLiteral("size"), QJsonValue(pointSize));
    fontConfig.insert(QStringLiteral("normal"), normalFont);
    root.insert(QStringLiteral("font"), fontConfig);

    QJsonObject themeConfig = root.value(QStringLiteral("theme")).toObject();
    QJsonObject lightConfig = themeConfig.value(QStringLiteral("light")).toObject();
    QJsonObject darkConfig = themeConfig.value(QStringLiteral("dark")).toObject();
    const QString iconTheme = m_kde->iconTheme().trimmed();
    if (!iconTheme.isEmpty())
    {
        lightConfig.insert(QStringLiteral("icon_theme"), iconTheme);
        darkConfig.insert(QStringLiteral("icon_theme"), iconTheme);
    }

    if (generatedThemes)
    {
        const QString lightThemeId = QString::fromLatin1(generatedVicinaeLightThemeId);
        const QString darkThemeId = QString::fromLatin1(generatedVicinaeDarkThemeId);
        lightConfig.insert(QStringLiteral("name"), lightThemeId);
        darkConfig.insert(QStringLiteral("name"), darkThemeId);

        QJsonObject activeConfig = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light
            ? lightConfig
            : darkConfig;
        activeConfig.insert(QStringLiteral("name"), useLightTheme ? lightThemeId : darkThemeId);
        if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light)
            lightConfig = activeConfig;
        else
            darkConfig = activeConfig;
    }

    themeConfig.insert(QStringLiteral("light"), lightConfig);
    themeConfig.insert(QStringLiteral("dark"), darkConfig);
    root.insert(QStringLiteral("theme"), themeConfig);

    QSaveFile output(path);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    const QByteArray serialized = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (output.write(serialized) != serialized.size())
        return false;

    return output.commit();
}

bool WallpaperColorsController::useLightVicinaeTheme(const MauiKit::AdaptivePalette &palette) const
{
    if (!m_theme)
        return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light;

    if (m_theme->adaptiveColorSchemeEnabled())
        return palette.backgroundColor.lightness() >= 128;

    switch (static_cast<MauiStyleType>(m_theme->styleType()))
    {
    case MauiStyleType::Light:
    case MauiStyleType::Inverted:
        return true;
    case MauiStyleType::Dark:
    case MauiStyleType::TrueBlack:
        return false;
    case MauiStyleType::Adaptive:
        return palette.backgroundColor.lightness() >= 128;
    case MauiStyleType::Auto:
    default:
        return QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Light;
    }
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
    settings.remove(QStringLiteral("SynchronizeKde"));
    settings.setValue(QStringLiteral("SynchronizeVicinae"), m_vicinaeSynchronizationEnabled);
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

QString WallpaperColorsController::generatedVicinaeThemePath(const QString &themeId) const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
        + QStringLiteral("/vicinae/themes/") + themeId + QStringLiteral(".toml");
}

QString WallpaperColorsController::vicinaeSettingsPath() const
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
        .filePath(QStringLiteral("vicinae/settings.json"));
}
