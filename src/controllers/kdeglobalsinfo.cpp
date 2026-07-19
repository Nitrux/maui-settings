#include "kdeglobalsinfo.h"

#include <QApplication>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QIcon>
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
    QFont font;
    font.fromString(value);
    return font;
}

QString KdeGlobalsInfo::fontToString(const QFont &font) const
{
    return font.toString();
}

QString KdeGlobalsInfo::fontLabel(const QString &value) const
{
    QFont font;
    if (!font.fromString(value))
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

QColor KdeGlobalsInfo::colorSchemePreviewColor(const QString &scheme) const
{
    const QString path = colorSchemeFilePath(scheme);
    if (path.isEmpty())
        return QColor(QStringLiteral("#26c6da"));

    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("Colors:Selection"));
    const QString focusColor = settings.value(QStringLiteral("DecorationFocus")).toString();
    if (!focusColor.isEmpty())
        return QColor(focusColor);

    const QString background = settings.value(QStringLiteral("BackgroundNormal")).toString();
    if (!background.isEmpty())
        return QColor(background);

    settings.endGroup();
    settings.beginGroup(QStringLiteral("Colors:Window"));
    const QString windowBackground = settings.value(QStringLiteral("BackgroundNormal")).toString();
    if (!windowBackground.isEmpty())
        return QColor(windowBackground);

    return QColor(QStringLiteral("#26c6da"));
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
