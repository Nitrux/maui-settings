#include "kdeglobalsinfo.h"

#include <QApplication>

#include <algorithm>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QIcon>
#include <QVariant>
#include <QVariantMap>
#include <QSettings>
#include <QStandardPaths>

namespace
{
QString systemDefaultFont()
{
    return QApplication::font().toString();
}

QString systemSmallFont()
{
    return QApplication::font().toString();
}

QString systemMonospaceFont()
{
    return QFontDatabase::systemFont(QFontDatabase::FixedFont).toString();
}

void addUnique(QStringList &list, const QString &value)
{
    const QString trimmed = value.trimmed();
    if (!trimmed.isEmpty() && !list.contains(trimmed))
        list.append(trimmed);
}

QColor readKdeColor(QSettings &settings, const QString &key, const QColor &fallback)
{
    const QVariant rawValue = settings.value(key);
    const QStringList valueParts = rawValue.toStringList();
    const QString value = (valueParts.size() > 1 ? valueParts.join(QLatin1Char(',')) : rawValue.toString()).trimmed();
    const QColor namedColor(value);
    if (namedColor.isValid())
        return namedColor;

    const QStringList components = value.split(QLatin1Char(','));
    if (components.size() < 3 || components.size() > 4)
        return fallback;

    int channels[4] = {0, 0, 0, 255};
    for (qsizetype index = 0; index < components.size(); ++index)
    {
        bool ok = false;
        const int channel = components.at(index).trimmed().toInt(&ok);
        if (!ok || channel < 0 || channel > 255)
            return fallback;

        channels[index] = channel;
    }

    return QColor(channels[0], channels[1], channels[2], channels[3]);
}

} // namespace

KdeGlobalsInfo::KdeGlobalsInfo(QObject *parent)
    : QObject(parent)
    , m_configPath(QDir::homePath() + QStringLiteral("/.config/kdeglobals"))
{
    load();
}

QString KdeGlobalsInfo::configPath() const
{
    return m_configPath;
}

QString KdeGlobalsInfo::colorScheme() const
{
    return m_colorScheme;
}

QString KdeGlobalsInfo::iconTheme() const
{
    return m_iconTheme;
}

QString KdeGlobalsInfo::defaultFont() const
{
    return m_defaultFont;
}

QString KdeGlobalsInfo::smallFont() const
{
    return m_smallFont;
}

QString KdeGlobalsInfo::monospaceFont() const
{
    return m_monospaceFont;
}

QStringList KdeGlobalsInfo::colorSchemes() const
{
    return m_colorSchemes;
}

QStringList KdeGlobalsInfo::iconThemes() const
{
    return m_iconThemes;
}

void KdeGlobalsInfo::setChanged()
{
    Q_EMIT settingsChanged();
}

void KdeGlobalsInfo::setColorScheme(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_colorScheme == normalized)
        return;

    m_colorScheme = normalized;
    setChanged();
}

void KdeGlobalsInfo::setIconTheme(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_iconTheme == normalized)
        return;

    m_iconTheme = normalized;
    setChanged();
}

void KdeGlobalsInfo::setDefaultFont(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_defaultFont == normalized)
        return;

    m_defaultFont = normalized;
    setChanged();
}

void KdeGlobalsInfo::setSmallFont(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_smallFont == normalized)
        return;

    m_smallFont = normalized;
    setChanged();
}

void KdeGlobalsInfo::setMonospaceFont(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_monospaceFont == normalized)
        return;

    m_monospaceFont = normalized;
    setChanged();
}

void KdeGlobalsInfo::reload()
{
    load();
}

bool KdeGlobalsInfo::save()
{
    QSettings settings(m_configPath, QSettings::IniFormat);

    settings.beginGroup(QStringLiteral("General"));
    settings.setValue(QStringLiteral("ColorScheme"), m_colorScheme);
    settings.setValue(QStringLiteral("font"), m_defaultFont);
    settings.setValue(QStringLiteral("menuFont"), m_defaultFont);
    settings.setValue(QStringLiteral("toolBarFont"), m_defaultFont);
    settings.setValue(QStringLiteral("smallestReadableFont"), m_smallFont);
    settings.setValue(QStringLiteral("fixed"), m_monospaceFont);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("KDE"));
    settings.setValue(QStringLiteral("ColorScheme"), m_colorScheme);
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Icons"));
    settings.setValue(QStringLiteral("Theme"), m_iconTheme);
    settings.endGroup();

    settings.sync();
    return settings.status() == QSettings::NoError;
}

QFont KdeGlobalsInfo::fontFromString(const QString &value) const
{
    const QString description = value.trimmed();
    if (description.isEmpty())
        return QApplication::font();

    QFont font;
    font.fromString(description);
    return font;
}

QString KdeGlobalsInfo::fontToString(const QFont &font) const
{
    return font.toString();
}

QString KdeGlobalsInfo::fontLabel(const QString &value) const
{
    const QString description = value.trimmed();
    if (description.isEmpty())
        return {};

    QFont font;
    if (!font.fromString(description))
        return value.trimmed();

    const QString family = font.family().trimmed();
    const int size = font.pointSize() > 0 ? font.pointSize() : font.pixelSize();
    const QString sizeText = size > 0 ? QString::number(size) : QString();
    const QString style = font.styleName().trimmed();

    if (family.isEmpty())
        return value.trimmed();

    if (!sizeText.isEmpty() && !style.isEmpty())
        return QStringLiteral("%1, %2, %3").arg(family, sizeText, style);

    if (!sizeText.isEmpty())
        return QStringLiteral("%1, %2").arg(family, sizeText);

    return family;
}


QVariantList KdeGlobalsInfo::iconThemePreviewIcons(const QString &theme) const
{
    QVariantList icons;
    const QString normalized = theme.trimmed();
    if (normalized.isEmpty())
        return icons;

    const QString previousTheme = QIcon::themeName();
    QIcon::setThemeName(normalized);

    const QStringList iconNames = {
        QStringLiteral("folder"),
        QStringLiteral("document-open"),
        QStringLiteral("preferences-desktop-icons"),
        QStringLiteral("applications-system"),
        QStringLiteral("media-playback-start"),
        QStringLiteral("network-wireless"),
        QStringLiteral("image-x-generic"),
        QStringLiteral("system-run")
    };

    for (const QString &iconName : iconNames)
    {
        QIcon icon = QIcon::fromTheme(iconName);
        if (icon.isNull())
            icon = QIcon::fromTheme(QStringLiteral("image-missing"));

        int nativeSize = 64;
        const QList<QSize> availableSizes = icon.availableSizes();
        if (!availableSizes.isEmpty())
        {
            nativeSize = 0;
            for (const QSize &availableSize : availableSizes)
            {
                const int candidate = qMin(availableSize.width(), availableSize.height());
                if (candidate <= 64)
                    nativeSize = qMax(nativeSize, candidate);
            }

            if (nativeSize == 0)
                nativeSize = 64;
        }

        QVariantMap previewIcon;
        previewIcon.insert(QStringLiteral("icon"), QVariant::fromValue(icon));
        previewIcon.insert(QStringLiteral("size"), nativeSize);
        icons << previewIcon;
    }

    QIcon::setThemeName(previousTheme);

    std::stable_sort(icons.begin(), icons.end(), [](const QVariant &left, const QVariant &right)
    {
        return left.toMap().value(QStringLiteral("size")).toInt() < right.toMap().value(QStringLiteral("size")).toInt();
    });

    return icons;
}

QVariantMap KdeGlobalsInfo::colorSchemePreview(const QString &scheme) const
{
    QVariantMap preview;
    const QString normalized = scheme.trimmed();
    if (normalized.isEmpty())
        return preview;

    QColor windowBackground(QStringLiteral("#232334"));
    QColor windowForeground(QStringLiteral("#f2f2f7"));
    QColor viewBackground(QStringLiteral("#1f1f2d"));
    QColor viewForeground(QStringLiteral("#f2f2f7"));
    QColor linkForeground(QStringLiteral("#26c6da"));
    QColor visitedForeground(QStringLiteral("#b39ddb"));
    QColor inactiveForeground(QStringLiteral("#888899"));
    QColor selectionBackground(QStringLiteral("#26c6da"));
    QColor selectionForeground(QStringLiteral("#ffffff"));
    QColor selectionLinkForeground(QStringLiteral("#26c6da"));
    QColor selectionVisitedForeground(QStringLiteral("#b39ddb"));
    QColor buttonBackground(QStringLiteral("#3a3a4d"));
    QColor buttonForeground(QStringLiteral("#f2f2f7"));
    QColor focusColor(QStringLiteral("#26c6da"));

    const QString filePath = colorSchemeFilePath(normalized);
    if (!filePath.isEmpty())
    {
        QSettings settings(filePath, QSettings::IniFormat);

        settings.beginGroup(QStringLiteral("Colors:Window"));
        windowBackground = readKdeColor(settings, QStringLiteral("BackgroundNormal"), windowBackground);
        windowForeground = readKdeColor(settings, QStringLiteral("ForegroundNormal"), windowForeground);
        focusColor = readKdeColor(settings, QStringLiteral("DecorationFocus"), focusColor);
        settings.endGroup();

        settings.beginGroup(QStringLiteral("Colors:View"));
        viewBackground = readKdeColor(settings, QStringLiteral("BackgroundNormal"), viewBackground);
        viewForeground = readKdeColor(settings, QStringLiteral("ForegroundNormal"), viewForeground);
        linkForeground = readKdeColor(settings, QStringLiteral("ForegroundLink"), linkForeground);
        visitedForeground = readKdeColor(settings, QStringLiteral("ForegroundVisited"), visitedForeground);
        inactiveForeground = readKdeColor(settings, QStringLiteral("ForegroundInactive"), inactiveForeground);
        settings.endGroup();

        settings.beginGroup(QStringLiteral("Colors:Selection"));
        selectionBackground = readKdeColor(settings, QStringLiteral("BackgroundNormal"), selectionBackground);
        selectionForeground = readKdeColor(settings, QStringLiteral("ForegroundNormal"), selectionForeground);
        selectionLinkForeground = readKdeColor(settings, QStringLiteral("ForegroundLink"), selectionLinkForeground);
        selectionVisitedForeground = readKdeColor(settings, QStringLiteral("ForegroundVisited"), selectionVisitedForeground);
        settings.endGroup();

        settings.beginGroup(QStringLiteral("Colors:Button"));
        buttonBackground = readKdeColor(settings, QStringLiteral("BackgroundNormal"), buttonBackground);
        buttonForeground = readKdeColor(settings, QStringLiteral("ForegroundNormal"), buttonForeground);
        settings.endGroup();
    }

    preview.insert(QStringLiteral("windowBackground"), windowBackground.name());
    preview.insert(QStringLiteral("windowForeground"), windowForeground.name());
    preview.insert(QStringLiteral("viewBackground"), viewBackground.name());
    preview.insert(QStringLiteral("viewForeground"), viewForeground.name());
    preview.insert(QStringLiteral("linkForeground"), linkForeground.name());
    preview.insert(QStringLiteral("visitedForeground"), visitedForeground.name());
    preview.insert(QStringLiteral("inactiveForeground"), inactiveForeground.name());
    preview.insert(QStringLiteral("selectionBackground"), selectionBackground.name());
    preview.insert(QStringLiteral("selectionForeground"), selectionForeground.name());
    preview.insert(QStringLiteral("selectionLinkForeground"), selectionLinkForeground.name());
    preview.insert(QStringLiteral("selectionVisitedForeground"), selectionVisitedForeground.name());
    preview.insert(QStringLiteral("buttonBackground"), buttonBackground.name());
    preview.insert(QStringLiteral("buttonForeground"), buttonForeground.name());
    preview.insert(QStringLiteral("focusColor"), focusColor.name());
    preview.insert(QStringLiteral("colors"), QVariantList {
        windowBackground.name(),
        viewBackground.name(),
        buttonBackground.name(),
        selectionBackground.name(),
        windowForeground.name(),
        selectionForeground.name()
    });

    return preview;
}

QString KdeGlobalsInfo::colorSchemeFilePath(const QString &scheme) const
{
    const QString trimmed = scheme.trimmed();
    if (trimmed.isEmpty())
        return {};

    const QString fileName = trimmed.endsWith(QStringLiteral(".colors")) ? trimmed : trimmed + QStringLiteral(".colors");
    const QStringList locations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &basePath : locations)
    {
        const QString candidate = basePath + QStringLiteral("/color-schemes/") + fileName;
        if (QFileInfo::exists(candidate))
            return candidate;
    }

    return {};
}

QStringList KdeGlobalsInfo::scanColorSchemes() const
{
    QStringList schemes;
    const QStringList locations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &basePath : locations)
    {
        const QDir dir(basePath + QStringLiteral("/color-schemes"));
        const QFileInfoList files = dir.entryInfoList(QStringList() << QStringLiteral("*.colors"), QDir::Files | QDir::Readable | QDir::NoSymLinks, QDir::Name);
        for (const QFileInfo &info : files)
            addUnique(schemes, info.completeBaseName());
    }

    addUnique(schemes, m_colorScheme);
    return schemes;
}

QStringList KdeGlobalsInfo::scanIconThemes() const
{
    QStringList themes;
    const QStringList locations = QIcon::themeSearchPaths();
    for (const QString &basePath : locations)
    {
        const QDir dir(basePath);
        const QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
        for (const QFileInfo &info : entries)
        {
            const QString indexThemePath = info.absoluteFilePath() + QStringLiteral("/index.theme");
            if (QFileInfo::exists(indexThemePath))
                addUnique(themes, info.fileName());
        }
    }

    addUnique(themes, m_iconTheme);
    return themes;
}

void KdeGlobalsInfo::load()
{
    m_colorScheme.clear();
    m_iconTheme.clear();
    m_defaultFont.clear();
    m_smallFont.clear();
    m_monospaceFont.clear();

    QSettings settings(m_configPath, QSettings::IniFormat);

    settings.beginGroup(QStringLiteral("General"));
    m_colorScheme = settings.value(QStringLiteral("ColorScheme")).toString();
    m_defaultFont = settings.value(QStringLiteral("font"), systemDefaultFont()).toString();
    m_smallFont = settings.value(QStringLiteral("smallestReadableFont"), systemSmallFont()).toString();
    m_monospaceFont = settings.value(QStringLiteral("fixed"), systemMonospaceFont()).toString();
    settings.endGroup();

    settings.beginGroup(QStringLiteral("KDE"));
    if (m_colorScheme.isEmpty())
        m_colorScheme = settings.value(QStringLiteral("ColorScheme")).toString();
    settings.endGroup();

    settings.beginGroup(QStringLiteral("Icons"));
    m_iconTheme = settings.value(QStringLiteral("Theme")).toString();
    settings.endGroup();

    if (m_defaultFont.isEmpty())
        m_defaultFont = systemDefaultFont();
    if (m_smallFont.isEmpty())
        m_smallFont = systemSmallFont();
    if (m_monospaceFont.isEmpty())
        m_monospaceFont = systemMonospaceFont();

    m_colorSchemes = scanColorSchemes();
    m_iconThemes = scanIconThemes();

    setChanged();
}
