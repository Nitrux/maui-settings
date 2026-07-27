#include "kdeglobalsinfo.h"

#include <QApplication>

#include <algorithm>
#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QIcon>
#include <QImage>
#include <QLocale>
#include <QSet>
#include <QVariant>
#include <QVariantMap>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleHints>

#include <QFile>
#include <QMap>
#include <QRegularExpression>
#include <QSaveFile>

#include <X11/Xcursor/Xcursor.h>

namespace
{
struct IniSection
{
    QString name;
    QStringList lines;
};

using IniUpdates = QMap<QString, QMap<QString, QString>>;

QString normalizedIniName(const QString &name)
{
    QString normalized = QString::fromUtf8(QByteArray::fromPercentEncoding(name.trimmed().toUtf8()));
    if (normalized == QLatin1String("%General"))
        normalized = QStringLiteral("General");

    if (normalized.startsWith(QStringLiteral("Colors:A")))
        normalized.remove(7, 1);
    else if (normalized.startsWith(QStringLiteral("ColorEffects:A")))
        normalized.remove(13, 1);

    return normalized;
}

bool splitIniEntry(const QString &line, QString *key, QString *value)
{
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#')) || trimmed.startsWith(QLatin1Char(';')))
        return false;

    const qsizetype equalsIndex = line.indexOf(QLatin1Char('='));
    if (equalsIndex <= 0)
        return false;

    *key = line.left(equalsIndex).trimmed();
    *value = line.mid(equalsIndex + 1).trimmed();
    return true;
}

QString normalizedIniValue(const QString &value)
{
    QString candidate = value.trimmed();
    const bool quoted = candidate.size() >= 2 && candidate.startsWith(QLatin1Char('"')) && candidate.endsWith(QLatin1Char('"'));
    if (quoted)
        candidate = candidate.mid(1, candidate.size() - 2);

    static const QRegularExpression serializedListPattern(
        QStringLiteral("^[^,=]+(?:,\\s*-?\\d+(?:\\.\\d+)?)+$"));
    if (!serializedListPattern.match(candidate).hasMatch())
        return value;

    candidate.replace(QRegularExpression(QStringLiteral(",\\s+")), QStringLiteral(","));
    return candidate;
}

qsizetype entryIndex(const IniSection &section, const QString &key)
{
    for (qsizetype index = 0; index < section.lines.size(); ++index)
    {
        QString existingKey;
        QString existingValue;
        if (splitIniEntry(section.lines.at(index), &existingKey, &existingValue)
            && normalizedIniName(existingKey) == key)
        {
            return index;
        }
    }

    return -1;
}

QString readIniValue(const QString &path, const QString &group, const QString &key, const QString &fallback = {})
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return fallback;

    QString currentGroup;
    QString result = fallback;
    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
    for (const QString &line : lines)
    {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith(QLatin1Char('[')) && trimmed.endsWith(QLatin1Char(']')))
        {
            currentGroup = normalizedIniName(trimmed.mid(1, trimmed.size() - 2));
            continue;
        }

        QString entryKey;
        QString entryValue;
        if (currentGroup == group && splitIniEntry(line, &entryKey, &entryValue)
            && normalizedIniName(entryKey) == key)
        {
            result = normalizedIniValue(entryValue);
        }
    }

    return result;
}

bool writeIniValues(const QString &path, const IniUpdates &updates)
{
    QFile input(path);
    QByteArray contents;
    if (input.exists())
    {
        if (!input.open(QIODevice::ReadOnly))
            return false;
        contents = input.readAll();
    }

    QList<IniSection> sections(1);
    QMap<QString, qsizetype> sectionIndexes;
    qsizetype currentSection = 0;
    bool mergingDuplicate = false;

    const QStringList sourceLines = QString::fromUtf8(contents).split(QLatin1Char('\n'));
    for (const QString &line : sourceLines)
    {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith(QLatin1Char('[')) && trimmed.endsWith(QLatin1Char(']')))
        {
            const QString sectionName = normalizedIniName(trimmed.mid(1, trimmed.size() - 2));
            const auto existingSection = sectionIndexes.constFind(sectionName);
            if (existingSection != sectionIndexes.cend())
            {
                currentSection = existingSection.value();
                mergingDuplicate = true;
            }
            else
            {
                currentSection = sections.size();
                sectionIndexes.insert(sectionName, currentSection);
                sections.append(IniSection {sectionName, {}});
                mergingDuplicate = false;
            }
            continue;
        }

        IniSection &section = sections[currentSection];
        QString entryKey;
        QString entryValue;
        if (splitIniEntry(line, &entryKey, &entryValue))
        {
            entryKey = normalizedIniName(entryKey);
            entryValue = normalizedIniValue(entryValue);
            if (entryKey == QLatin1String("update_info"))
                entryValue.replace(QRegularExpression(QStringLiteral(",\\s+")), QStringLiteral(","));
            const QString normalizedLine = entryKey + QLatin1Char('=') + entryValue;
            const qsizetype existingEntry = entryIndex(section, entryKey);
            if (mergingDuplicate && existingEntry >= 0)
                section.lines[existingEntry] = normalizedLine;
            else
                section.lines.append(normalizedLine);
        }
        else if (!mergingDuplicate)
        {
            section.lines.append(line);
        }
    }

    if (contents.isEmpty())
        sections[0].lines.clear();

    for (auto groupIterator = updates.cbegin(); groupIterator != updates.cend(); ++groupIterator)
    {
        qsizetype sectionIndex = sectionIndexes.value(groupIterator.key(), -1);
        if (sectionIndex < 0)
        {
            sectionIndex = sections.size();
            sectionIndexes.insert(groupIterator.key(), sectionIndex);
            sections.append(IniSection {groupIterator.key(), {}});
        }

        IniSection &section = sections[sectionIndex];
        for (auto entryIterator = groupIterator.value().cbegin(); entryIterator != groupIterator.value().cend(); ++entryIterator)
        {
            const QString line = entryIterator.key() + QLatin1Char('=') + entryIterator.value();
            const qsizetype index = entryIndex(section, entryIterator.key());
            if (index >= 0)
                section.lines[index] = line;
            else
                section.lines.append(line);
        }
    }

    QStringList outputLines = sections.constFirst().lines;
    for (qsizetype index = 1; index < sections.size(); ++index)
    {
        if (!outputLines.isEmpty() && !outputLines.constLast().isEmpty())
            outputLines.append(QString());
        outputLines.append(QLatin1Char('[') + sections.at(index).name + QLatin1Char(']'));
        outputLines.append(sections.at(index).lines);
    }

    QSaveFile output(path);
    if (!output.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    if (output.write(outputLines.join(QLatin1Char('\n')).toUtf8()) < 0)
        return false;
    return output.commit();
}

bool iniBoolean(const QString &value, bool fallback)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QLatin1String("true") || normalized == QLatin1String("1")
        || normalized == QLatin1String("yes") || normalized == QLatin1String("on"))
    {
        return true;
    }
    if (normalized == QLatin1String("false") || normalized == QLatin1String("0")
        || normalized == QLatin1String("no") || normalized == QLatin1String("off"))
    {
        return false;
    }
    return fallback;
}

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

QStringList iniStringList(const QVariant &value)
{
    QStringList values = value.toStringList();
    if (values.size() == 1 && values.constFirst().contains(QLatin1Char(',')))
        values = values.constFirst().split(QLatin1Char(','), Qt::SkipEmptyParts);

    for (QString &entry : values)
        entry = entry.trimmed();

    values.removeAll(QString());
    return values;
}

QString localizedThemeName(QSettings &settings, const QString &fallback)
{
    const QLocale locale = QLocale::system();
    const QString localeName = locale.name();
    const QString languageName = localeName.section(QLatin1Char('_'), 0, 0);
    const QStringList keys = {
        QStringLiteral("Name[%1]").arg(localeName),
        QStringLiteral("Name[%1]").arg(languageName),
        QStringLiteral("Name")
    };

    for (const QString &key : keys)
    {
        const QString name = settings.value(key).toString().trimmed();
        if (!name.isEmpty())
            return name;
    }

    return fallback;
}

QVariantList scanThemeOptions(bool cursorThemes)
{
    QVariantList themes;
    QSet<QString> seenThemes;

    QStringList locations;
    addUnique(locations, QDir::homePath() + QStringLiteral("/.icons"));

    const QStringList dataLocations = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &dataLocation : dataLocations)
        addUnique(locations, dataLocation + QStringLiteral("/icons"));

    const QStringList iconSearchPaths = QIcon::themeSearchPaths();
    for (const QString &iconSearchPath : iconSearchPaths)
        addUnique(locations, iconSearchPath);

    for (const QString &basePath : locations)
    {
        const QDir baseDirectory(basePath);
        const QFileInfoList entries = baseDirectory.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &entry : entries)
        {
            const QString themeId = entry.fileName();
            if (seenThemes.contains(themeId))
                continue;

            const QString indexPath = entry.absoluteFilePath() + QStringLiteral("/index.theme");
            if (!QFileInfo::exists(indexPath))
                continue;

            QSettings metadata(indexPath, QSettings::IniFormat);
            metadata.beginGroup(QStringLiteral("Icon Theme"));
            const QString baseName = metadata.value(QStringLiteral("Name")).toString().trimmed();
            const bool hidden = metadata.value(QStringLiteral("Hidden"), false).toBool();
            const bool hasIconDirectories = !iniStringList(metadata.value(QStringLiteral("Directories"))).isEmpty()
                || !iniStringList(metadata.value(QStringLiteral("ScaledDirectories"))).isEmpty();
            const bool hasInheritedThemes = !iniStringList(metadata.value(QStringLiteral("Inherits"))).isEmpty();
            const QString displayName = localizedThemeName(metadata, baseName);
            metadata.endGroup();

            if (baseName.isEmpty())
                continue;

            const bool hasCursorDirectory = QDir(entry.absoluteFilePath() + QStringLiteral("/cursors")).exists();
            const bool isCursorTheme = hasCursorDirectory || (!hasIconDirectories && hasInheritedThemes);
            if (hidden || (cursorThemes ? !isCursorTheme : !hasIconDirectories))
                continue;

            seenThemes.insert(themeId);

            QVariantMap option;
            option.insert(QStringLiteral("label"), displayName);
            option.insert(QStringLiteral("value"), themeId);
            themes << option;
        }
    }

    std::stable_sort(themes.begin(), themes.end(), [](const QVariant &left, const QVariant &right)
    {
        return QString::localeAwareCompare(left.toMap().value(QStringLiteral("label")).toString(),
                                           right.toMap().value(QStringLiteral("label")).toString()) < 0;
    });

    return themes;
}

QImage loadCursorImage(const QString &theme, const QString &cursorName, int requestedSize)
{
    const QByteArray themeName = theme.toUtf8();
    const QByteArray name = cursorName.toUtf8();
    XcursorImage *cursor = XcursorLibraryLoadImage(name.constData(), themeName.constData(), requestedSize);
    if (!cursor || !cursor->pixels || cursor->width == 0 || cursor->height == 0)
    {
        if (cursor)
            XcursorImageDestroy(cursor);
        return {};
    }

    const QImage image(reinterpret_cast<const uchar *>(cursor->pixels),
                       static_cast<int>(cursor->width),
                       static_cast<int>(cursor->height),
                       QImage::Format_ARGB32);
    const QImage detachedImage = image.copy();
    XcursorImageDestroy(cursor);
    return detachedImage;
}

} // namespace

KdeGlobalsInfo::KdeGlobalsInfo(QObject *parent)
    : QObject(parent)
    , m_configPath(QDir::homePath() + QStringLiteral("/.config/kdeglobals"))
    , m_inputConfigPath(QDir::homePath() + QStringLiteral("/.config/kcminputrc"))
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

QString KdeGlobalsInfo::cursorTheme() const
{
    return m_cursorTheme;
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

bool KdeGlobalsInfo::singleClick() const
{
    return m_singleClick;
}

QStringList KdeGlobalsInfo::colorSchemes() const
{
    return m_colorSchemes;
}

QStringList KdeGlobalsInfo::iconThemes() const
{
    return m_iconThemes;
}

QStringList KdeGlobalsInfo::iconThemeIds() const
{
    return m_iconThemeIds;
}

QStringList KdeGlobalsInfo::cursorThemes() const
{
    return m_cursorThemes;
}

QStringList KdeGlobalsInfo::cursorThemeIds() const
{
    return m_cursorThemeIds;
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

void KdeGlobalsInfo::setCursorTheme(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_cursorTheme == normalized)
        return;

    m_cursorTheme = normalized;
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

void KdeGlobalsInfo::setSingleClick(bool value)
{
    if (m_singleClick == value)
        return;

    m_singleClick = value;
    setChanged();
}

void KdeGlobalsInfo::reload()
{
    load();
}

bool KdeGlobalsInfo::save()
{
    IniUpdates settingsUpdates;
    QMap<QString, QString> &generalGroup = settingsUpdates[QStringLiteral("General")];
    generalGroup.insert(QStringLiteral("ColorScheme"), m_colorScheme);
    generalGroup.insert(QStringLiteral("font"), m_defaultFont);
    generalGroup.insert(QStringLiteral("menuFont"), m_defaultFont);
    generalGroup.insert(QStringLiteral("toolBarFont"), m_defaultFont);
    generalGroup.insert(QStringLiteral("smallestReadableFont"), m_smallFont);
    generalGroup.insert(QStringLiteral("fixed"), m_monospaceFont);

    QMap<QString, QString> &kdeGroup = settingsUpdates[QStringLiteral("KDE")];
    kdeGroup.insert(QStringLiteral("ColorScheme"), m_colorScheme);
    kdeGroup.insert(QStringLiteral("SingleClick"), m_singleClick ? QStringLiteral("true") : QStringLiteral("false"));

    settingsUpdates[QStringLiteral("Icons")].insert(QStringLiteral("Theme"), m_iconTheme);
    const bool settingsSaved = writeIniValues(m_configPath, settingsUpdates);

    IniUpdates inputUpdates;
    inputUpdates[QStringLiteral("Mouse")].insert(QStringLiteral("cursorTheme"), m_cursorTheme);
    const bool inputSettingsSaved = writeIniValues(m_inputConfigPath, inputUpdates);

    return settingsSaved && inputSettingsSaved;
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

        const QImage renderedIcon = icon.pixmap(nativeSize, nativeSize).toImage();
        if (renderedIcon.isNull())
            continue;

        QVariantMap previewIcon;
        previewIcon.insert(QStringLiteral("icon"), renderedIcon);
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

QVariantList KdeGlobalsInfo::cursorThemePreviewImages(const QString &theme) const
{
    QVariantList cursors;
    const QString normalized = theme.trimmed();
    if (normalized.isEmpty())
        return cursors;

    const QStringList cursorNames = {
        QStringLiteral("left_ptr"),
        QStringLiteral("hand2"),
        QStringLiteral("xterm"),
        QStringLiteral("crosshair"),
        QStringLiteral("watch"),
        QStringLiteral("size_all")
    };

    for (const QString &cursorName : cursorNames)
    {
        const QImage image = loadCursorImage(normalized, cursorName, 48);
        if (image.isNull())
            continue;

        QVariantMap previewCursor;
        previewCursor.insert(QStringLiteral("image"), image);
        previewCursor.insert(QStringLiteral("width"), image.width());
        previewCursor.insert(QStringLiteral("height"), image.height());
        cursors << previewCursor;
    }

    return cursors;
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

QVariantList KdeGlobalsInfo::scanIconThemes() const
{
    return scanThemeOptions(false);
}

QVariantList KdeGlobalsInfo::scanCursorThemes() const
{
    return scanThemeOptions(true);
}

void KdeGlobalsInfo::load()
{
    m_colorScheme.clear();
    m_iconTheme.clear();
    m_cursorTheme.clear();
    m_defaultFont.clear();
    m_smallFont.clear();
    m_monospaceFont.clear();

    m_colorScheme = readIniValue(m_configPath, QStringLiteral("General"), QStringLiteral("ColorScheme"));
    m_defaultFont = readIniValue(m_configPath, QStringLiteral("General"), QStringLiteral("font"), systemDefaultFont());
    m_smallFont = readIniValue(m_configPath, QStringLiteral("General"), QStringLiteral("smallestReadableFont"), systemSmallFont());
    m_monospaceFont = readIniValue(m_configPath, QStringLiteral("General"), QStringLiteral("fixed"), systemMonospaceFont());

    if (m_colorScheme.isEmpty())
        m_colorScheme = readIniValue(m_configPath, QStringLiteral("KDE"), QStringLiteral("ColorScheme"));
    m_singleClick = iniBoolean(
        readIniValue(m_configPath, QStringLiteral("KDE"), QStringLiteral("SingleClick")),
        QApplication::styleHints()->singleClickActivation());

    m_iconTheme = readIniValue(m_configPath, QStringLiteral("Icons"), QStringLiteral("Theme"));
    m_cursorTheme = readIniValue(
        m_inputConfigPath,
        QStringLiteral("Mouse"),
        QStringLiteral("cursorTheme"),
        qEnvironmentVariable("XCURSOR_THEME"));

    if (m_defaultFont.isEmpty())
        m_defaultFont = systemDefaultFont();
    if (m_smallFont.isEmpty())
        m_smallFont = systemSmallFont();
    if (m_monospaceFont.isEmpty())
        m_monospaceFont = systemMonospaceFont();

    m_colorSchemes = scanColorSchemes();
    m_iconThemes.clear();
    m_iconThemeIds.clear();
    const QVariantList iconThemeOptions = scanIconThemes();
    for (const QVariant &optionValue : iconThemeOptions)
    {
        const QVariantMap option = optionValue.toMap();
        m_iconThemes.append(option.value(QStringLiteral("label")).toString());
        m_iconThemeIds.append(option.value(QStringLiteral("value")).toString());
    }

    m_cursorThemes.clear();
    m_cursorThemeIds.clear();
    const QVariantList cursorThemeOptions = scanCursorThemes();
    for (const QVariant &optionValue : cursorThemeOptions)
    {
        const QVariantMap option = optionValue.toMap();
        m_cursorThemes.append(option.value(QStringLiteral("label")).toString());
        m_cursorThemeIds.append(option.value(QStringLiteral("value")).toString());
    }

    setChanged();
}
