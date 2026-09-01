#pragma once

#include <QObject>
#include <QColor>
#include <QFont>
#include <QImage>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class KdeGlobalsInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(QString widgetStyle READ widgetStyle WRITE setWidgetStyle NOTIFY settingsChanged)
    Q_PROPERTY(QString colorScheme READ colorScheme WRITE setColorScheme NOTIFY settingsChanged)
    Q_PROPERTY(QString iconTheme READ iconTheme WRITE setIconTheme NOTIFY settingsChanged)
    Q_PROPERTY(QString cursorTheme READ cursorTheme WRITE setCursorTheme NOTIFY settingsChanged)
    Q_PROPERTY(int cursorSize READ cursorSize WRITE setCursorSize NOTIFY settingsChanged)
    Q_PROPERTY(QString defaultFont READ defaultFont WRITE setDefaultFont NOTIFY settingsChanged)
    Q_PROPERTY(QString menuFont READ menuFont WRITE setMenuFont NOTIFY settingsChanged)
    Q_PROPERTY(QString toolBarFont READ toolBarFont WRITE setToolBarFont NOTIFY settingsChanged)
    Q_PROPERTY(QString smallFont READ smallFont WRITE setSmallFont NOTIFY settingsChanged)
    Q_PROPERTY(QString monospaceFont READ monospaceFont WRITE setMonospaceFont NOTIFY settingsChanged)
    Q_PROPERTY(QString fontHinting READ fontHinting WRITE setFontHinting NOTIFY settingsChanged)
    Q_PROPERTY(QString fontAntialiasing READ fontAntialiasing WRITE setFontAntialiasing NOTIFY settingsChanged)
    Q_PROPERTY(QString fontRgbaOrder READ fontRgbaOrder WRITE setFontRgbaOrder NOTIFY settingsChanged)
    Q_PROPERTY(bool singleClick READ singleClick WRITE setSingleClick NOTIFY settingsChanged)
    Q_PROPERTY(QStringList colorSchemes READ colorSchemes NOTIFY settingsChanged)
    Q_PROPERTY(QStringList iconThemes READ iconThemes NOTIFY settingsChanged)
    Q_PROPERTY(QStringList iconThemeIds READ iconThemeIds NOTIFY settingsChanged)
    Q_PROPERTY(QStringList cursorThemes READ cursorThemes NOTIFY settingsChanged)
    Q_PROPERTY(QStringList cursorThemeIds READ cursorThemeIds NOTIFY settingsChanged)
    Q_PROPERTY(QStringList widgetStyles READ widgetStyles NOTIFY settingsChanged)
    Q_PROPERTY(QStringList widgetStyleIds READ widgetStyleIds NOTIFY settingsChanged)

public:
    explicit KdeGlobalsInfo(QObject *parent = nullptr);

    QString configPath() const;
    QString widgetStyle() const;
    QString colorScheme() const;
    QString iconTheme() const;
    QString cursorTheme() const;
    int cursorSize() const;
    QString defaultFont() const;
    QString menuFont() const;
    QString toolBarFont() const;
    QString smallFont() const;
    QString monospaceFont() const;
    QString fontHinting() const;
    QString fontAntialiasing() const;
    QString fontRgbaOrder() const;
    bool singleClick() const;
    QStringList colorSchemes() const;
    QStringList iconThemes() const;
    QStringList iconThemeIds() const;
    QStringList cursorThemes() const;
    QStringList cursorThemeIds() const;
    QStringList widgetStyles() const;
    QStringList widgetStyleIds() const;

    void setColorScheme(const QString &value);
    void setWidgetStyle(const QString &value);
    void setIconTheme(const QString &value);
    void setCursorTheme(const QString &value);
    void setCursorSize(int value);
    void setDefaultFont(const QString &value);
    void setMenuFont(const QString &value);
    void setToolBarFont(const QString &value);
    void setSmallFont(const QString &value);
    void setMonospaceFont(const QString &value);
    void setFontHinting(const QString &value);
    void setFontAntialiasing(const QString &value);
    void setFontRgbaOrder(const QString &value);
    void setSingleClick(bool value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();
    Q_INVOKABLE bool applyColorSchemeFile(const QString &path, const QString &scheme);

    Q_INVOKABLE QFont fontFromString(const QString &value) const;
    Q_INVOKABLE QString fontToString(const QFont &font) const;
    Q_INVOKABLE QString fontLabel(const QString &value) const;
    Q_INVOKABLE QVariantMap colorSchemePreview(const QString &scheme) const;
    Q_INVOKABLE QVariantMap widgetStylePreview(const QString &style) const;
    Q_INVOKABLE QString colorSchemeFilePath(const QString &scheme) const;
    Q_INVOKABLE QVariantList iconThemePreviewIcons(const QString &theme) const;
    Q_INVOKABLE QVariantList cursorThemePreviewImages(const QString &theme) const;

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    void setChanged();
    QStringList scanColorSchemes() const;
    QStringList scanWidgetStyles() const;
    QVariantList scanIconThemes() const;
    QVariantList scanCursorThemes() const;

private:
    QString m_configPath;
    QString m_inputConfigPath;
    QString m_widgetStyle;
    QString m_colorScheme;
    QString m_iconTheme;
    QString m_cursorTheme;
    int m_cursorSize = 24;
    QString m_defaultFont;
    QString m_menuFont;
    QString m_toolBarFont;
    QString m_smallFont;
    QString m_monospaceFont;
    QString m_fontHinting = QStringLiteral("slight");
    QString m_fontAntialiasing = QStringLiteral("grayscale");
    QString m_fontRgbaOrder = QStringLiteral("rgb");
    bool m_singleClick = true;
    QStringList m_colorSchemes;
    QStringList m_iconThemes;
    QStringList m_iconThemeIds;
    QStringList m_cursorThemes;
    QStringList m_cursorThemeIds;
    QStringList m_widgetStyles;
    QStringList m_widgetStyleIds;
};
