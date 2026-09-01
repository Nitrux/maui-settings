#include "kdeglobalsinfo.h"

#include <QApplication>
#include <QGuiApplication>
#include <QWindow>
#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KJob>

#include <algorithm>
#include <utility>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QPalette>
#include <QRegularExpression>
#include <QPixmapCache>
#include <QLocale>
#include <QSet>
#include <QVariant>
#include <QVariantMap>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QStyle>
#include <QStyleHints>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <KColorScheme>
#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#include <X11/Xcursor/Xcursor.h>

namespace
{
constexpr auto greeterHelperId = "org.maui.settings.qmlgreet";
constexpr auto greeterCopyActionId = "org.maui.settings.qmlgreet.copykdeglobals";
bool authHelperAvailable(const QString &serviceName)
{
    QDBusConnectionInterface *busInterface = QDBusConnection::systemBus().interface();
    if (!busInterface)
        return false;

    const QDBusReply<bool> registered = busInterface->isServiceRegistered(serviceName);
    const QDBusReply<QStringList> activatable = busInterface->activatableServiceNames();
    return (registered.isValid() && registered.value())
        || (activatable.isValid() && activatable.value().contains(serviceName));
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

QString xftHintStyle(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("none"))
        return QStringLiteral("hintnone");
    if (normalized == QStringLiteral("medium"))
        return QStringLiteral("hintmedium");
    if (normalized == QStringLiteral("full"))
        return QStringLiteral("hintfull");
    return QStringLiteral("hintslight");
}

QString kdeHintingValue(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized.startsWith(QStringLiteral("hint")))
        return normalized.mid(4);
    if (normalized == QStringLiteral("none") || normalized == QStringLiteral("medium") || normalized == QStringLiteral("full"))
        return normalized;
    return QStringLiteral("slight");
}

void setWidgetStyleRecursively(QWidget *widget, QStyle *style, const QPalette &palette)
{
    widget->setPalette(QPalette());
    widget->setPalette(palette);
    widget->setStyle(style);

    for (QObject *child : widget->children())
    {
        if (child->isWidgetType())
            setWidgetStyleRecursively(static_cast<QWidget *>(child), style, palette);
    }
}

bool parseFontDescription(const QString &value, QFont &font)
{
    const QString description = value.trimmed();
    if (description.isEmpty())
        return false;

    // GTK stores Pango descriptions such as "Noto Sans Bold 10", while
    // kdeglobals stores QFont::toString() values containing commas.
    if (!description.contains(QLatin1Char(44)))
    {
        static const QRegularExpression gtkFontPattern(QStringLiteral(R"(^(.+?)\s+([0-9]+(?:\.[0-9]+)?)$)"));
        const QRegularExpressionMatch match = gtkFontPattern.match(description);
        if (match.hasMatch())
        {
            const QString familyAndStyle = match.captured(1).trimmed();
            bool sizeOk = false;
            const qreal pointSize = match.captured(2).toDouble(&sizeOk);
            if (!sizeOk || pointSize <= 0)
                return false;

            const QFontDatabase database;
            QString family = familyAndStyle;
            QString style;
            QStringList families = database.families();
            std::sort(families.begin(), families.end(), [](const QString &left, const QString &right) {
                return left.size() > right.size();
            });

            for (const QString &candidate : std::as_const(families))
            {
                if (familyAndStyle.compare(candidate, Qt::CaseInsensitive) == 0)
                {
                    family = candidate;
                    break;
                }

                const QString prefix = candidate + QLatin1Char(32);
                if (!familyAndStyle.startsWith(prefix, Qt::CaseInsensitive))
                    continue;

                const QString candidateStyle = familyAndStyle.mid(prefix.size()).trimmed();
                const QStringList styles = database.styles(candidate);
                const bool knownStyle = std::any_of(styles.cbegin(), styles.cend(), [&candidateStyle](const QString &value) {
                    return value.compare(candidateStyle, Qt::CaseInsensitive) == 0;
                });
                if (knownStyle)
                {
                    family = candidate;
                    style = candidateStyle;
                    break;
                }
            }

            font.setFamily(family);
            font.setPointSizeF(pointSize);
            if (!style.isEmpty())
                font.setStyleName(style);
            return true;
        }
    }

    return font.fromString(description);
}

QFont configFont(const QString &value)
{
    QFont font;
    return parseFontDescription(value, font) ? font : QApplication::font();
}

void notifyKcmChange(int changeType)
{
    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KGlobalSettings"),
                                                      QStringLiteral("org.kde.KGlobalSettings"),
                                                      QStringLiteral("notifyChange"));
    message.setArguments({changeType, 0});
    QDBusConnection::sessionBus().send(message);
}

bool applyColorScheme(KConfig *target, const QString &path)
{
    if (path.isEmpty())
        return false;

    const KSharedConfigPtr source = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    const QStringList colorGroups {
        QStringLiteral("Colors:View"),
        QStringLiteral("Colors:Window"),
        QStringLiteral("Colors:Button"),
        QStringLiteral("Colors:Selection"),
        QStringLiteral("Colors:Tooltip"),
        QStringLiteral("Colors:Complementary"),
        QStringLiteral("Colors:Header"),
        QStringLiteral("ColorEffects:Inactive"),
        QStringLiteral("ColorEffects:Disabled")
    };

    for (const QString &groupName : colorGroups)
    {
        KConfigGroup targetGroup(target, groupName);
        targetGroup.deleteGroup();

        KConfigGroup sourceGroup(source, groupName);
        if (sourceGroup.exists())
            sourceGroup.copyTo(&targetGroup);
    }

    const QStringList windowManagerColorKeys {
        QStringLiteral("activeBackground"),
        QStringLiteral("activeForeground"),
        QStringLiteral("inactiveBackground"),
        QStringLiteral("inactiveForeground"),
        QStringLiteral("activeBlend"),
        QStringLiteral("inactiveBlend")
    };
    KConfigGroup sourceWindowManager(source, QStringLiteral("WM"));
    KConfigGroup targetWindowManager(target, QStringLiteral("WM"));
    for (const QString &key : windowManagerColorKeys)
    {
        if (sourceWindowManager.hasKey(key))
            targetWindowManager.writeEntry(key, sourceWindowManager.readEntry(key, QString()));
    }

    KConfigGroup sourceKde(source, QStringLiteral("KDE"));
    KConfigGroup targetKde(target, QStringLiteral("KDE"));
    for (const QString &key : {QStringLiteral("frameContrast"), QStringLiteral("contrast")})
    {
        if (sourceKde.hasKey(key))
            targetKde.writeEntry(key, sourceKde.readEntry(key, QString()));
    }

    QFile colorSchemeFile(path);
    if (colorSchemeFile.open(QIODevice::ReadOnly))
    {
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(&colorSchemeFile);
        KConfigGroup(target, QStringLiteral("General")).writeEntry(QStringLiteral("ColorSchemeHash"), QString::fromLatin1(hash.result().toHex()));
    }

    return true;
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

QString KdeGlobalsInfo::widgetStyle() const
{
    return m_widgetStyle;
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

int KdeGlobalsInfo::cursorSize() const
{
    return m_cursorSize;
}

QString KdeGlobalsInfo::defaultFont() const
{
    return m_defaultFont;
}

QString KdeGlobalsInfo::menuFont() const
{
    return m_menuFont;
}

QString KdeGlobalsInfo::toolBarFont() const
{
    return m_toolBarFont;
}

QString KdeGlobalsInfo::smallFont() const
{
    return m_smallFont;
}

QString KdeGlobalsInfo::monospaceFont() const
{
    return m_monospaceFont;
}

QString KdeGlobalsInfo::fontHinting() const
{
    return m_fontHinting;
}

QString KdeGlobalsInfo::fontAntialiasing() const
{
    return m_fontAntialiasing;
}

QString KdeGlobalsInfo::fontRgbaOrder() const
{
    return m_fontRgbaOrder;
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

QStringList KdeGlobalsInfo::widgetStyles() const
{
    return m_widgetStyles;
}

QStringList KdeGlobalsInfo::widgetStyleIds() const
{
    return m_widgetStyleIds;
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

void KdeGlobalsInfo::setWidgetStyle(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_widgetStyle == normalized)
        return;

    m_widgetStyle = normalized;
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

void KdeGlobalsInfo::setCursorSize(int value)
{
    const int normalized = qBound(0, value, 1024);
    if (m_cursorSize == normalized)
        return;

    m_cursorSize = normalized;
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

void KdeGlobalsInfo::setMenuFont(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_menuFont == normalized)
        return;

    m_menuFont = normalized;
    setChanged();
}

void KdeGlobalsInfo::setToolBarFont(const QString &value)
{
    const QString normalized = value.trimmed();
    if (m_toolBarFont == normalized)
        return;

    m_toolBarFont = normalized;
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

void KdeGlobalsInfo::setFontHinting(const QString &value)
{
    const QString normalized = kdeHintingValue(value);
    if (m_fontHinting == normalized)
        return;
    m_fontHinting = normalized;
    setChanged();
}

void KdeGlobalsInfo::setFontAntialiasing(const QString &value)
{
    const QString normalized = value.trimmed().toLower() == QStringLiteral("none") ? QStringLiteral("none") : QStringLiteral("grayscale");
    if (m_fontAntialiasing == normalized)
        return;
    m_fontAntialiasing = normalized;
    setChanged();
}

void KdeGlobalsInfo::setFontRgbaOrder(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    const QStringList validValues {QStringLiteral("none"), QStringLiteral("rgb"), QStringLiteral("bgr"), QStringLiteral("vrgb"), QStringLiteral("vbgr")};
    const QString result = validValues.contains(normalized) ? normalized : QStringLiteral("rgb");
    if (m_fontRgbaOrder == result)
        return;
    m_fontRgbaOrder = result;
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

bool KdeGlobalsInfo::applyColorSchemeFile(const QString &path, const QString &scheme)
{
    if (path.trimmed().isEmpty() || QFileInfo(path).isFile() == false || scheme.trimmed().isEmpty())
        return false;

    const KSharedConfigPtr settings = KSharedConfig::openConfig(m_configPath, KConfig::SimpleConfig);
    KConfigGroup generalGroup(settings, QStringLiteral("General"));
    generalGroup.writeEntry(QStringLiteral("ColorScheme"), scheme.trimmed());
    if (applyColorScheme(settings.data(), path) == false)
        return false;

    settings->sync();
    if (settings->isDirty())
        return false;

    load();
    notifyKcmChange(0);
    return true;
}

bool KdeGlobalsInfo::save()
{
    const KSharedConfigPtr settings = KSharedConfig::openConfig(m_configPath, KConfig::SimpleConfig);
    KConfigGroup generalGroup(settings, QStringLiteral("General"));
    const bool paletteChanged = generalGroup.readEntry(QStringLiteral("ColorScheme"), QString()) != m_colorScheme;
    if (paletteChanged)
    {
        generalGroup.writeEntry(QStringLiteral("ColorScheme"), m_colorScheme);
        applyColorScheme(settings.data(), colorSchemeFilePath(m_colorScheme));
    }

    const bool fontRenderingChanged = generalGroup.readEntry(QStringLiteral("XftHintStyle"), QStringLiteral("hintslight")) != xftHintStyle(m_fontHinting)
        || generalGroup.readEntry(QStringLiteral("XftAntialias"), true) != (m_fontAntialiasing != QStringLiteral("none"))
        || generalGroup.readEntry(QStringLiteral("XftSubPixel"), QStringLiteral("rgb")) != m_fontRgbaOrder;
    if (fontRenderingChanged)
    {
        generalGroup.writeEntry(QStringLiteral("XftHintStyle"), xftHintStyle(m_fontHinting));
        generalGroup.writeEntry(QStringLiteral("XftAntialias"), m_fontAntialiasing != QStringLiteral("none"));
        generalGroup.writeEntry(QStringLiteral("XftSubPixel"), m_fontRgbaOrder);
    }

    const auto writeFontIfChanged = [&generalGroup](const QString &key, const QString &value) {
        const QFont desired = configFont(value);
        const QFont current = generalGroup.readEntry(key, QFont());
        if (generalGroup.hasKey(key) && current == desired)
            return false;

        generalGroup.writeEntry(key, desired);
        return true;
    };
    writeFontIfChanged(QStringLiteral("font"), m_defaultFont);
    writeFontIfChanged(QStringLiteral("menuFont"), m_menuFont);
    writeFontIfChanged(QStringLiteral("toolBarFont"), m_toolBarFont);
    writeFontIfChanged(QStringLiteral("smallestReadableFont"), m_smallFont);
    writeFontIfChanged(QStringLiteral("fixed"), m_monospaceFont);

    KConfigGroup kdeGroup(settings, QStringLiteral("KDE"));
    const bool widgetStyleChanged = kdeGroup.readEntry(QStringLiteral("widgetStyle"), QString()) != m_widgetStyle;
    if (widgetStyleChanged)
        kdeGroup.writeEntry(QStringLiteral("widgetStyle"), m_widgetStyle);

    if (kdeGroup.readEntry(QStringLiteral("SingleClick"), QApplication::styleHints()->singleClickActivation()) != m_singleClick)
        kdeGroup.writeEntry(QStringLiteral("SingleClick"), m_singleClick);

    KConfigGroup iconsGroup(settings, QStringLiteral("Icons"));
    if (iconsGroup.readEntry(QStringLiteral("Theme"), QString()) != m_iconTheme)
        iconsGroup.writeEntry(QStringLiteral("Theme"), m_iconTheme);
    settings->sync();

    const KSharedConfigPtr inputSettings = KSharedConfig::openConfig(m_inputConfigPath, KConfig::SimpleConfig);
    KConfigGroup mouseGroup(inputSettings, QStringLiteral("Mouse"));
    const bool cursorThemeChanged = mouseGroup.readEntry(QStringLiteral("cursorTheme"), qEnvironmentVariable("XCURSOR_THEME")) != m_cursorTheme;
    const bool cursorSizeChanged = mouseGroup.readEntry(QStringLiteral("cursorSize"), 24) != m_cursorSize;
    if (cursorThemeChanged)
        mouseGroup.writeEntry(QStringLiteral("cursorTheme"), m_cursorTheme);
    if (cursorSizeChanged)
        mouseGroup.writeEntry(QStringLiteral("cursorSize"), m_cursorSize);
    inputSettings->sync();

    const bool settingsSaved = !settings->isDirty();
    const bool inputSettingsSaved = !inputSettings->isDirty();
    if (settingsSaved && inputSettingsSaved)
    {
        if (paletteChanged || fontRenderingChanged)
            notifyKcmChange(0); // PaletteChanged
        if (widgetStyleChanged)
            notifyKcmChange(2); // StyleChanged
        if (cursorThemeChanged || cursorSizeChanged)
            notifyKcmChange(5); // CursorChanged
    }

    if (settingsSaved && inputSettingsSaved && authHelperAvailable(QString::fromLatin1(greeterHelperId)))
    {
        KAuth::Action action(QString::fromLatin1(greeterCopyActionId));
        action.setHelperId(QString::fromLatin1(greeterHelperId));
        action.setArguments({{QStringLiteral("sourcePath"), m_configPath}});
        if (QWindow *window = QGuiApplication::focusWindow())
            action.setParentWindow(window);

        if (action.isValid())
        {
            if (KAuth::ExecuteJob *job = action.execute())
            {
                connect(job, &KJob::result, this, [](KJob *completedJob)
                {
                    if (completedJob->error() != 0)
                        qWarning() << "Could not copy kdeglobals to the greetd user:" << completedJob->errorText();
                });
                job->start();
            }
        }
    }

    return settingsSaved && inputSettingsSaved;
}

QFont KdeGlobalsInfo::fontFromString(const QString &value) const
{
    const QString description = value.trimmed();
    if (description.isEmpty())
        return QApplication::font();

    return configFont(description);
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
    if (!parseFontDescription(description, font))
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


QVariantMap KdeGlobalsInfo::widgetStylePreview(const QString &style) const
{
    QVariantMap preview;
    const QString normalized = style.trimmed();
    if (normalized.isEmpty())
        return preview;

    QStyle *widgetStyle = QStyleFactory::create(normalized);
    if (!widgetStyle)
        return preview;

    QWidget widget;
    widget.setAttribute(Qt::WA_DontShowOnScreen);
    widget.setAttribute(Qt::WA_QuitOnClose, false);
    widget.setAutoFillBackground(true);
    widget.setWindowTitle(QStringLiteral("Widget style preview"));

    auto *layout = new QVBoxLayout(&widget);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(10);

    auto *title = new QLabel(QStringLiteral("Qt Widgets preview"), &widget);
    title->setStyleSheet(QString());
    layout->addWidget(title);

    auto *lineEdit = new QLineEdit(QStringLiteral("Text field"), &widget);
    layout->addWidget(lineEdit);

    auto *controls = new QHBoxLayout;
    auto *button = new QPushButton(QStringLiteral("Button"), &widget);
    auto *checkBox = new QCheckBox(QStringLiteral("Check box"), &widget);
    auto *comboBox = new QComboBox(&widget);
    comboBox->addItems({QStringLiteral("Option one"), QStringLiteral("Option two")});
    controls->addWidget(button);
    controls->addWidget(checkBox);
    controls->addWidget(comboBox);
    layout->addLayout(controls);

    auto *slider = new QSlider(Qt::Horizontal, &widget);
    slider->setValue(60);
    layout->addWidget(slider);

    widget.resize(460, 190);

    QPixmapCache::clear();
    QPalette palette(KColorScheme::createApplicationPalette(KSharedConfig::openConfig()));
    widgetStyle->polish(palette);
    for (int i = 0; i < QPalette::NColorRoles; ++i)
    {
        const auto role = static_cast<QPalette::ColorRole>(i);
        palette.setColor(QPalette::Inactive, role, palette.color(QPalette::Active, role));
    }

    setWidgetStyleRecursively(&widget, widgetStyle, palette);
    widget.ensurePolished();
    widget.show();

    QImage image(widget.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(widget.palette().window().color());
    QPainter painter(&image);
    widget.render(&painter);
    painter.end();
    widget.hide();

    preview.insert(QStringLiteral("image"), image);
    preview.insert(QStringLiteral("width"), image.width());
    preview.insert(QStringLiteral("height"), image.height());
    return preview;
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

QStringList KdeGlobalsInfo::scanWidgetStyles() const
{
    QStringList styles = QStyleFactory::keys();
    styles.sort(Qt::CaseInsensitive);
    if (!m_widgetStyle.isEmpty() && !styles.contains(m_widgetStyle, Qt::CaseInsensitive))
        styles.append(m_widgetStyle);
    return styles;
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
    m_widgetStyle.clear();
    m_iconTheme.clear();
    m_cursorTheme.clear();
    m_cursorSize = 24;
    m_defaultFont.clear();
    m_menuFont.clear();
    m_toolBarFont.clear();
    m_smallFont.clear();
    m_monospaceFont.clear();
    m_fontHinting = QStringLiteral("slight");
    m_fontAntialiasing = QStringLiteral("grayscale");
    m_fontRgbaOrder = QStringLiteral("rgb");

    const KSharedConfigPtr settings = KSharedConfig::openConfig(m_configPath, KConfig::SimpleConfig);
    const KConfigGroup generalGroup(settings, QStringLiteral("General"));
    m_colorScheme = generalGroup.readEntry(QStringLiteral("ColorScheme"), QString());
    m_defaultFont = generalGroup.readEntry(QStringLiteral("font"), systemDefaultFont());
    m_menuFont = generalGroup.readEntry(QStringLiteral("menuFont"), m_defaultFont);
    m_toolBarFont = generalGroup.readEntry(QStringLiteral("toolBarFont"), m_defaultFont);
    m_smallFont = generalGroup.readEntry(QStringLiteral("smallestReadableFont"), systemSmallFont());
    m_monospaceFont = generalGroup.readEntry(QStringLiteral("fixed"), systemMonospaceFont());
    m_fontHinting = kdeHintingValue(generalGroup.readEntry(QStringLiteral("XftHintStyle"), QStringLiteral("hintslight")));
    m_fontAntialiasing = generalGroup.readEntry(QStringLiteral("XftAntialias"), true) ? QStringLiteral("grayscale") : QStringLiteral("none");
    m_fontRgbaOrder = generalGroup.readEntry(QStringLiteral("XftSubPixel"), QStringLiteral("rgb")).trimmed().toLower();
    const QStringList rgbaValues {QStringLiteral("none"), QStringLiteral("rgb"), QStringLiteral("bgr"), QStringLiteral("vrgb"), QStringLiteral("vbgr")};
    if (!rgbaValues.contains(m_fontRgbaOrder))
        m_fontRgbaOrder = QStringLiteral("rgb");

    if (m_colorScheme.isEmpty())
        m_colorScheme = settings->group(QStringLiteral("KDE")).readEntry(QStringLiteral("ColorScheme"), QString());
    const KConfigGroup kdeGroup(settings, QStringLiteral("KDE"));
    m_widgetStyle = kdeGroup.readEntry(QStringLiteral("widgetStyle"), QString());
    m_singleClick = kdeGroup.readEntry(QStringLiteral("SingleClick"), QApplication::styleHints()->singleClickActivation());

    m_iconTheme = settings->group(QStringLiteral("Icons")).readEntry(QStringLiteral("Theme"), QString());
    const KSharedConfigPtr inputSettings = KSharedConfig::openConfig(m_inputConfigPath, KConfig::SimpleConfig);
    const KConfigGroup mouseGroup(inputSettings, QStringLiteral("Mouse"));
    m_cursorTheme = mouseGroup.readEntry(QStringLiteral("cursorTheme"), qEnvironmentVariable("XCURSOR_THEME"));
    m_cursorSize = qBound(0, mouseGroup.readEntry(QStringLiteral("cursorSize"), 24), 1024);

    if (m_defaultFont.isEmpty())
        m_defaultFont = systemDefaultFont();
    if (m_menuFont.isEmpty())
        m_menuFont = m_defaultFont;
    if (m_toolBarFont.isEmpty())
        m_toolBarFont = m_defaultFont;
    if (m_smallFont.isEmpty())
        m_smallFont = systemSmallFont();
    if (m_monospaceFont.isEmpty())
        m_monospaceFont = systemMonospaceFont();

    if (m_widgetStyle.isEmpty())
        m_widgetStyle = QApplication::style() ? QApplication::style()->objectName() : QStringLiteral("Fusion");

    const QStringList availableStyles = QStyleFactory::keys();
    for (const QString &style : availableStyles)
    {
        if (style.compare(m_widgetStyle, Qt::CaseInsensitive) == 0)
        {
            m_widgetStyle = style;
            break;
        }
    }

    m_colorSchemes = scanColorSchemes();
    m_widgetStyleIds = scanWidgetStyles();
    m_widgetStyles = m_widgetStyleIds;
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
