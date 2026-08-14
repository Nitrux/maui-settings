#include "gtksettingsinfo.h"

#include <QApplication>
#include <QCoreApplication>
#include <QImage>
#include <QList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QIcon>
#include <QMap>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPair>
#include <QSaveFile>
#include <QSettings>
#include <QSet>
#include <QStandardPaths>
#include <QVariantMap>

#include <algorithm>

namespace
{
QString configHome()
{
    const QString value = qEnvironmentVariable("XDG_CONFIG_HOME").trimmed();
    return value.isEmpty() ? QDir::homePath() + QStringLiteral("/.config") : value;
}

void addUnique(QStringList &list, const QString &value)
{
    const QString normalized = value.trimmed();
    if (!normalized.isEmpty() && !list.contains(normalized))
        list.append(normalized);
}

QString findPreviewHelper(const QString &name)
{
    const QString localPath = QDir(QCoreApplication::applicationDirPath()).filePath(name);
    if (QFileInfo(localPath).isExecutable())
        return localPath;

    return QStandardPaths::findExecutable(name);
}

QImage renderThemePreview(const QString &helper, const QString &theme)
{
    if (helper.isEmpty() || theme.trimmed().isEmpty())
        return {};

    QProcess process;
    process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    process.start(helper, {
        QStringLiteral("--theme"), theme,
        QStringLiteral("--width"), QStringLiteral("460"),
        QStringLiteral("--height"), QStringLiteral("190")
    });

    if (!process.waitForFinished(5000)
        || process.exitStatus() != QProcess::NormalExit
        || process.exitCode() != 0)
        return {};

    QImage image;
    image.loadFromData(process.readAllStandardOutput(), "PNG");
    return image;
}

QString gsettingsValue(const QString &schema, const QString &key, const QString &fallback)
{
    QProcess process;
    process.start(QStringLiteral("gsettings"), {QStringLiteral("get"), schema, key});
    if (!process.waitForFinished(1000) || process.exitCode() != 0)
        return fallback;

    QString value = QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed();
    if (value.size() >= 2 && value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\'')))
        value = value.mid(1, value.size() - 2);
    return value.isEmpty() ? fallback : value;
}

bool gsettingsBool(const QString &schema, const QString &key, bool fallback)
{
    const QString value = gsettingsValue(schema, key, fallback ? QStringLiteral("true") : QStringLiteral("false"));
    return value == QStringLiteral("true") || value == QStringLiteral("1");
}

void setGsettingsValue(const QString &schema, const QString &key, const QString &value)
{
    QProcess process;
    process.start(QStringLiteral("gsettings"), {QStringLiteral("set"), schema, key, value});
    process.waitForFinished(1000);
}

QString settingValue(const QString &path, const QString &key, const QString &fallback)
{
    if (!QFileInfo::exists(path))
        return fallback;

    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("Settings"));
    return settings.value(key, fallback).toString().trimmed();
}

QString gtkRcValue(const QString &path, const QString &key, const QString &fallback)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return fallback;

    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char(10));
    for (const QString &line : lines)
    {
        const int equals = line.indexOf(QLatin1Char(61));
        if (equals < 0 || line.left(equals).trimmed() != key)
            continue;

        QString value = line.mid(equals + 1).trimmed();
        if (value.size() >= 2 && value.startsWith(QLatin1Char(34)) && value.endsWith(QLatin1Char(34)))
            value = value.mid(1, value.size() - 2);
        return value.trimmed();
    }

    return fallback;
}

bool settingBool(const QString &path, const QString &key, bool fallback)
{
    const QString value = settingValue(path, key, fallback ? QStringLiteral("1") : QStringLiteral("0"));
    return value == QStringLiteral("1") || value.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0;
}

QStringList preservedSettings(const QString &path, const QMap<QString, QString> &values)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QStringList preserved;
    bool inSettings = false;
    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
    for (const QString &line : lines)
    {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith(QLatin1Char('[')) && trimmed.endsWith(QLatin1Char(']')))
        {
            inSettings = trimmed.compare(QStringLiteral("[Settings]"), Qt::CaseInsensitive) == 0;
            if (!inSettings)
                preserved.append(line);
            continue;
        }

        if (!inSettings || trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#')) || trimmed.startsWith(QLatin1Char(';')))
        {
            preserved.append(line);
            continue;
        }

        const int equals = line.indexOf(QLatin1Char('='));
        const QString key = equals < 0 ? QString() : line.left(equals).trimmed();
        if (!values.contains(key))
            preserved.append(line);
    }

    while (!preserved.isEmpty() && preserved.constLast().isEmpty())
        preserved.removeLast();
    return preserved;
}

bool writeGtkSettings(const QString &path, const QMap<QString, QString> &values)
{
    QDir().mkpath(QFileInfo(path).absolutePath());

    QStringList lines {QStringLiteral("[Settings]")};
    for (auto it = values.constBegin(); it != values.constEnd(); ++it)
        lines.append(it.key() + QLatin1Char('=') + it.value());

    const QStringList preserved = preservedSettings(path, values);
    if (!preserved.isEmpty())
    {
        lines.append(QString());
        lines.append(preserved);
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    file.write(lines.join(QLatin1Char('\n')).toUtf8());
    file.write("\n");
    return file.commit();
}

QVariantMap themeOption(const QString &path)
{
    QSettings metadata(path + QStringLiteral("/index.theme"), QSettings::IniFormat);
    metadata.beginGroup(QStringLiteral("Icon Theme"));
    const QString id = QFileInfo(path).fileName();
    const QString label = metadata.value(QStringLiteral("Name"), id).toString().trimmed();
    metadata.endGroup();

    return {{QStringLiteral("id"), id}, {QStringLiteral("label"), label.isEmpty() ? id : label}};
}

QStringList themeLocations()
{
    QStringList locations {QDir::homePath() + QStringLiteral("/.themes")};
    for (const QString &base : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation))
        addUnique(locations, base + QStringLiteral("/themes"));
    return locations;
}

QStringList iconLocations()
{
    QStringList locations {QDir::homePath() + QStringLiteral("/.icons")};
    for (const QString &base : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation))
        addUnique(locations, base + QStringLiteral("/icons"));
    for (const QString &path : QIcon::themeSearchPaths())
        addUnique(locations, path);
    return locations;
}

void appendOptions(const QStringList &locations, bool cursor, QStringList &labels, QStringList &ids)
{
    QSet<QString> seen;
    for (const QString &location : locations)
    {
        const QDir directory(location);
        const QFileInfoList entries = directory.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &entry : entries)
        {
            const QString path = entry.absoluteFilePath();
            const bool hasCursor = QDir(path + QStringLiteral("/cursors")).exists();
            const bool hasIndex = QFileInfo::exists(path + QStringLiteral("/index.theme"));
            if ((!cursor && !hasIndex) || (cursor && !hasCursor) || seen.contains(entry.fileName()))
                continue;

            seen.insert(entry.fileName());
            const QVariantMap option = themeOption(path);
            labels.append(option.value(QStringLiteral("label")).toString());
            ids.append(option.value(QStringLiteral("id")).toString());
        }
    }

    QList<int> order;
    for (int i = 0; i < labels.size(); ++i)
        order.append(i);
    std::sort(order.begin(), order.end(), [&labels](int left, int right) {
        return QString::localeAwareCompare(labels.at(left), labels.at(right)) < 0;
    });

    QStringList sortedLabels;
    QStringList sortedIds;
    for (const int index : order)
    {
        sortedLabels.append(labels.at(index));
        sortedIds.append(ids.at(index));
    }
    labels = sortedLabels;
    ids = sortedIds;
}

void applyGsettings(const QString &theme, const QString &iconTheme, const QString &font, const QString &cursorTheme,
                    int cursorSize, const QString &colorScheme, bool eventSounds, bool inputFeedbackSounds,
                    const QString &fontHinting, const QString &fontAntialiasing, const QString &fontRgbaOrder,
                    double textScalingFactor)
{
    setGsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("gtk-theme"), theme);
    setGsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("icon-theme"), iconTheme);
    setGsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("font-name"), font);
    setGsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("cursor-theme"), cursorTheme);
    setGsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("cursor-size"), QString::number(cursorSize));
    setGsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("color-scheme"), colorScheme);
    setGsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("font-hinting"), fontHinting);
    setGsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("font-antialiasing"), fontAntialiasing);
    setGsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("font-rgba-order"), fontRgbaOrder);
    setGsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("text-scaling-factor"), QString::number(textScalingFactor));
    setGsettingsValue(QStringLiteral("org.gnome.desktop.sound"), QStringLiteral("event-sounds"), eventSounds ? QStringLiteral("true") : QStringLiteral("false"));
    setGsettingsValue(QStringLiteral("org.gnome.desktop.sound"), QStringLiteral("input-feedback-sounds"), inputFeedbackSounds ? QStringLiteral("true") : QStringLiteral("false"));
}

QString hintStyle(const QString &hinting)
{
    if (hinting == QStringLiteral("slight"))
        return QStringLiteral("hintslight");
    if (hinting == QStringLiteral("full"))
        return QStringLiteral("hintfull");
    if (hinting == QStringLiteral("none"))
        return QStringLiteral("hintnone");
    return QStringLiteral("hintmedium");
}

bool writeGtkRc20(const QString &path, const QString &theme, const QString &iconTheme, const QString &font,
                 const QString &cursorTheme, int cursorSize, const QString &hinting,
                 const QString &rgba, bool eventSounds, bool inputFeedbackSounds)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    const QStringList lines {
        QStringLiteral("# Generated by maui-settings."),
        QStringLiteral("# Keep personal overrides in ~/.gtkrc-2.0.mine."),
        QStringLiteral("include ") + QLatin1Char('"') + QDir::homePath() + QStringLiteral("/.gtkrc-2.0.mine") + QLatin1Char('"'),
        QStringLiteral("gtk-theme-name=") + QLatin1Char('"') + theme + QLatin1Char('"'),
        QStringLiteral("gtk-icon-theme-name=") + QLatin1Char('"') + iconTheme + QLatin1Char('"'),
        QStringLiteral("gtk-font-name=") + QLatin1Char('"') + font + QLatin1Char('"'),
        QStringLiteral("gtk-cursor-theme-name=") + QLatin1Char('"') + cursorTheme + QLatin1Char('"'),
        QStringLiteral("gtk-cursor-theme-size=") + QString::number(cursorSize),
        QStringLiteral("gtk-enable-event-sounds=") + (eventSounds ? QStringLiteral("1") : QStringLiteral("0")),
        QStringLiteral("gtk-enable-input-feedback-sounds=") + (inputFeedbackSounds ? QStringLiteral("1") : QStringLiteral("0")),
        QStringLiteral("gtk-xft-antialias=") + (hinting == QStringLiteral("none") ? QStringLiteral("0") : QStringLiteral("1")),
        QStringLiteral("gtk-xft-hinting=") + (hinting == QStringLiteral("none") ? QStringLiteral("0") : QStringLiteral("1")),
        QStringLiteral("gtk-xft-hintstyle=") + QLatin1Char('"') + hintStyle(hinting) + QLatin1Char('"'),
        QStringLiteral("gtk-xft-rgba=") + QLatin1Char('"') + rgba + QLatin1Char('"')
    };

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    file.write(lines.join(QLatin1Char('\n')).toUtf8());
    file.write("\n");
    return file.commit();
}

} // namespace

GtkSettingsInfo::GtkSettingsInfo(QObject *parent)
    : QObject(parent)
{
    load();
}

QString GtkSettingsInfo::theme() const { return m_theme; }
QString GtkSettingsInfo::iconTheme() const { return m_iconTheme; }
QString GtkSettingsInfo::cursorTheme() const { return m_cursorTheme; }
QString GtkSettingsInfo::font() const { return m_font; }
int GtkSettingsInfo::cursorSize() const { return m_cursorSize; }
QString GtkSettingsInfo::colorScheme() const { return m_colorScheme; }
bool GtkSettingsInfo::eventSounds() const { return m_eventSounds; }
bool GtkSettingsInfo::inputFeedbackSounds() const { return m_inputFeedbackSounds; }
QString GtkSettingsInfo::fontHinting() const { return m_fontHinting; }
QString GtkSettingsInfo::fontAntialiasing() const { return m_fontAntialiasing; }
QString GtkSettingsInfo::fontRgbaOrder() const { return m_fontRgbaOrder; }
double GtkSettingsInfo::textScalingFactor() const { return m_textScalingFactor; }
QStringList GtkSettingsInfo::themes() const { return m_themes; }
QStringList GtkSettingsInfo::themeIds() const { return m_themeIds; }
QStringList GtkSettingsInfo::iconThemes() const { return m_iconThemes; }
QStringList GtkSettingsInfo::iconThemeIds() const { return m_iconThemeIds; }
QStringList GtkSettingsInfo::cursorThemes() const { return m_cursorThemes; }
QStringList GtkSettingsInfo::cursorThemeIds() const { return m_cursorThemeIds; }

void GtkSettingsInfo::setChanged() { Q_EMIT settingsChanged(); }

void GtkSettingsInfo::setTheme(const QString &value) { const QString normalized = value.trimmed(); if (m_theme == normalized) return; m_theme = normalized; setChanged(); }
void GtkSettingsInfo::setIconTheme(const QString &value) { const QString normalized = value.trimmed(); if (m_iconTheme == normalized) return; m_iconTheme = normalized; setChanged(); }
void GtkSettingsInfo::setCursorTheme(const QString &value) { const QString normalized = value.trimmed(); if (m_cursorTheme == normalized) return; m_cursorTheme = normalized; setChanged(); }
void GtkSettingsInfo::setFont(const QString &value) { const QString normalized = value.trimmed(); if (m_font == normalized) return; m_font = normalized; setChanged(); }
void GtkSettingsInfo::setColorScheme(const QString &value) { const QString normalized = value.trimmed(); if (m_colorScheme == normalized) return; m_colorScheme = normalized; setChanged(); }
void GtkSettingsInfo::setFontHinting(const QString &value) { const QString normalized = value.trimmed(); if (m_fontHinting == normalized) return; m_fontHinting = normalized; setChanged(); }
void GtkSettingsInfo::setFontAntialiasing(const QString &value) { const QString normalized = value.trimmed(); if (m_fontAntialiasing == normalized) return; m_fontAntialiasing = normalized; setChanged(); }
void GtkSettingsInfo::setFontRgbaOrder(const QString &value) { const QString normalized = value.trimmed(); if (m_fontRgbaOrder == normalized) return; m_fontRgbaOrder = normalized; setChanged(); }

void GtkSettingsInfo::setCursorSize(int value) { if (m_cursorSize == value) return; m_cursorSize = qBound(0, value, 1024); setChanged(); }
void GtkSettingsInfo::setEventSounds(bool value) { if (m_eventSounds == value) return; m_eventSounds = value; setChanged(); }
void GtkSettingsInfo::setInputFeedbackSounds(bool value) { if (m_inputFeedbackSounds == value) return; m_inputFeedbackSounds = value; setChanged(); }
void GtkSettingsInfo::setTextScalingFactor(double value) { if (qFuzzyCompare(m_textScalingFactor, value)) return; m_textScalingFactor = qBound(0.5, value, 3.0); setChanged(); }

void GtkSettingsInfo::reload() { load(); }

void GtkSettingsInfo::scanThemes()
{
    m_themes.clear();
    m_themeIds.clear();
    QSet<QString> seen;
    for (const QString &location : themeLocations())
    {
        const QDir directory(location);
        const QFileInfoList entries = directory.entryInfoList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &entry : entries)
        {
            if ((!QDir(entry.absoluteFilePath() + QStringLiteral("/gtk-3.0")).exists()
                 && !QDir(entry.absoluteFilePath() + QStringLiteral("/gtk-4.0")).exists())
                || seen.contains(entry.fileName()))
                continue;

            seen.insert(entry.fileName());
            const QVariantMap option = themeOption(entry.absoluteFilePath());
            m_themes.append(option.value(QStringLiteral("label")).toString());
            m_themeIds.append(option.value(QStringLiteral("id")).toString());
        }
    }

    QList<int> order;
    for (int i = 0; i < m_themes.size(); ++i)
        order.append(i);
    std::sort(order.begin(), order.end(), [this](int left, int right) {
        return QString::localeAwareCompare(m_themes.at(left), m_themes.at(right)) < 0;
    });

    QStringList labels;
    QStringList ids;
    for (const int index : order) { labels.append(m_themes.at(index)); ids.append(m_themeIds.at(index)); }
    m_themes = labels;
    m_themeIds = ids;
}

void GtkSettingsInfo::scanIconsAndCursors()
{
    m_iconThemes.clear();
    m_iconThemeIds.clear();
    appendOptions(iconLocations(), false, m_iconThemes, m_iconThemeIds);

    m_cursorThemes.clear();
    m_cursorThemeIds.clear();
    appendOptions(iconLocations(), true, m_cursorThemes, m_cursorThemeIds);
}

void GtkSettingsInfo::load()
{
    const QString gtk3 = configHome() + QStringLiteral("/gtk-3.0/settings.ini");
    const QString gtk4 = configHome() + QStringLiteral("/gtk-4.0/settings.ini");
    const QString gtk2 = QDir::homePath() + QStringLiteral("/.gtkrc-2.0");
    const QString themeFallback = gsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("gtk-theme"), QStringLiteral("Adwaita"));
    const QString iconFallback = gsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("icon-theme"), QIcon::themeName());
    const QString fontFallback = gsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("font-name"),
                                                 QApplication::font().family() + QLatin1Char(' ') + QString::number(qMax(1, QApplication::font().pointSize())));
    const QString cursorFallback = gsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("cursor-theme"), QString());
    const int fallbackCursorSize = gsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("cursor-size"), QStringLiteral("24")).toInt();

    m_theme = settingValue(gtk3, QStringLiteral("gtk-theme-name"), settingValue(gtk4, QStringLiteral("gtk-theme-name"), gtkRcValue(gtk2, QStringLiteral("gtk-theme-name"), themeFallback)));
    m_iconTheme = settingValue(gtk3, QStringLiteral("gtk-icon-theme-name"), settingValue(gtk4, QStringLiteral("gtk-icon-theme-name"), gtkRcValue(gtk2, QStringLiteral("gtk-icon-theme-name"), iconFallback)));
    m_font = settingValue(gtk3, QStringLiteral("gtk-font-name"), settingValue(gtk4, QStringLiteral("gtk-font-name"), gtkRcValue(gtk2, QStringLiteral("gtk-font-name"), fontFallback)));
    m_cursorTheme = settingValue(gtk3, QStringLiteral("gtk-cursor-theme-name"), settingValue(gtk4, QStringLiteral("gtk-cursor-theme-name"), gtkRcValue(gtk2, QStringLiteral("gtk-cursor-theme-name"), cursorFallback)));
    m_cursorSize = settingValue(gtk3, QStringLiteral("gtk-cursor-theme-size"), settingValue(gtk4, QStringLiteral("gtk-cursor-theme-size"), gtkRcValue(gtk2, QStringLiteral("gtk-cursor-theme-size"), QString::number(fallbackCursorSize)))).toInt();
    if (m_cursorSize < 0)
        m_cursorSize = fallbackCursorSize;

    const QString fileColorScheme = settingBool(gtk3, QStringLiteral("gtk-application-prefer-dark-theme"), false)
        ? QStringLiteral("prefer-dark") : QStringLiteral("default");
    m_colorScheme = gsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("color-scheme"), fileColorScheme);
    m_eventSounds = gsettingsBool(QStringLiteral("org.gnome.desktop.sound"), QStringLiteral("event-sounds"), true);
    m_inputFeedbackSounds = gsettingsBool(QStringLiteral("org.gnome.desktop.sound"), QStringLiteral("input-feedback-sounds"), false);
    m_fontHinting = gsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("font-hinting"), QStringLiteral("medium"));
    m_fontAntialiasing = gsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("font-antialiasing"), QStringLiteral("grayscale"));
    m_fontRgbaOrder = gsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("font-rgba-order"), QStringLiteral("rgb"));
    m_textScalingFactor = gsettingsValue(QStringLiteral("org.gnome.desktop.interface"), QStringLiteral("text-scaling-factor"), QStringLiteral("1.0")).toDouble();

    scanThemes();
    scanIconsAndCursors();
    setChanged();
}

QString GtkSettingsInfo::fontToString(const QFont &font) const
{
    QStringList parts {font.family()};
    const QString style = font.styleName().trimmed();
    if (!style.isEmpty() && style.compare(QStringLiteral("Regular"), Qt::CaseInsensitive) != 0)
        parts.append(style);

    const qreal pointSize = font.pointSizeF() > 0 ? font.pointSizeF() : font.pixelSize();
    if (pointSize > 0)
        parts.append(QString::number(pointSize));

    return parts.join(QLatin1Char(32));
}

QVariantList GtkSettingsInfo::gtkThemePreviews(const QString &theme) const
{
    QVariantList previews;
    const QString normalizedTheme = theme.trimmed();
    if (normalizedTheme.isEmpty())
        return previews;

    const QList<QPair<QString, QString>> helpers {
        {QStringLiteral("GTK 3"), QStringLiteral("maui-settings-gtk3-preview-helper")},
        {QStringLiteral("GTK 4"), QStringLiteral("maui-settings-gtk4-preview-helper")}
    };

    for (const auto &helper : helpers)
    {
        const QImage image = renderThemePreview(findPreviewHelper(helper.second), normalizedTheme);
        if (image.isNull())
            continue;

        previews.append(QVariantMap {
            {QStringLiteral("toolkit"), helper.first},
            {QStringLiteral("image"), image},
            {QStringLiteral("width"), image.width()},
            {QStringLiteral("height"), image.height()}
        });
    }

    return previews;
}

bool GtkSettingsInfo::save()
{
    QMap<QString, QString> values;
    values.insert(QStringLiteral("gtk-application-prefer-dark-theme"), m_colorScheme == QStringLiteral("prefer-dark") ? QStringLiteral("1") : QStringLiteral("0"));
    values.insert(QStringLiteral("gtk-cursor-theme-name"), m_cursorTheme);
    values.insert(QStringLiteral("gtk-cursor-theme-size"), QString::number(m_cursorSize));
    values.insert(QStringLiteral("gtk-enable-event-sounds"), m_eventSounds ? QStringLiteral("1") : QStringLiteral("0"));
    values.insert(QStringLiteral("gtk-enable-input-feedback-sounds"), m_inputFeedbackSounds ? QStringLiteral("1") : QStringLiteral("0"));
    values.insert(QStringLiteral("gtk-font-name"), m_font);
    values.insert(QStringLiteral("gtk-icon-theme-name"), m_iconTheme);
    values.insert(QStringLiteral("gtk-theme-name"), m_theme);
    values.insert(QStringLiteral("gtk-xft-antialias"), m_fontAntialiasing == QStringLiteral("none") ? QStringLiteral("0") : QStringLiteral("1"));
    values.insert(QStringLiteral("gtk-xft-hinting"), m_fontHinting == QStringLiteral("none") ? QStringLiteral("0") : QStringLiteral("1"));
    values.insert(QStringLiteral("gtk-xft-hintstyle"), hintStyle(m_fontHinting));
    values.insert(QStringLiteral("gtk-xft-rgba"), m_fontRgbaOrder);

    const bool saved3 = writeGtkSettings(configHome() + QStringLiteral("/gtk-3.0/settings.ini"), values);
    const bool saved2 = writeGtkRc20(QDir::homePath() + QStringLiteral("/.gtkrc-2.0"), m_theme, m_iconTheme, m_font, m_cursorTheme, m_cursorSize, m_fontHinting, m_fontRgbaOrder, m_eventSounds, m_inputFeedbackSounds);
    const QMap<QString, QString> values4 {
        {QStringLiteral("gtk-application-prefer-dark-theme"), values.value(QStringLiteral("gtk-application-prefer-dark-theme"))},
        {QStringLiteral("gtk-cursor-theme-name"), m_cursorTheme},
        {QStringLiteral("gtk-cursor-theme-size"), QString::number(m_cursorSize)},
        {QStringLiteral("gtk-font-name"), m_font},
        {QStringLiteral("gtk-icon-theme-name"), m_iconTheme},
        {QStringLiteral("gtk-theme-name"), m_theme}
    };
    const bool saved4 = writeGtkSettings(configHome() + QStringLiteral("/gtk-4.0/settings.ini"), values4);

    applyGsettings(m_theme, m_iconTheme, m_font, m_cursorTheme, m_cursorSize, m_colorScheme,
                   m_eventSounds, m_inputFeedbackSounds, m_fontHinting, m_fontAntialiasing,
                   m_fontRgbaOrder, m_textScalingFactor);
    return saved2 && saved3 && saved4;
}
