#pragma once

#include <QObject>
#include <QFont>
#include <QVariantList>
#include <QStringList>

class GtkSettingsInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY settingsChanged)
    Q_PROPERTY(QString iconTheme READ iconTheme WRITE setIconTheme NOTIFY settingsChanged)
    Q_PROPERTY(QString cursorTheme READ cursorTheme WRITE setCursorTheme NOTIFY settingsChanged)
    Q_PROPERTY(QString font READ font WRITE setFont NOTIFY settingsChanged)
    Q_PROPERTY(int cursorSize READ cursorSize WRITE setCursorSize NOTIFY settingsChanged)
    Q_PROPERTY(QString colorScheme READ colorScheme WRITE setColorScheme NOTIFY settingsChanged)
    Q_PROPERTY(bool eventSounds READ eventSounds WRITE setEventSounds NOTIFY settingsChanged)
    Q_PROPERTY(bool inputFeedbackSounds READ inputFeedbackSounds WRITE setInputFeedbackSounds NOTIFY settingsChanged)
    Q_PROPERTY(QString fontHinting READ fontHinting WRITE setFontHinting NOTIFY settingsChanged)
    Q_PROPERTY(QString fontAntialiasing READ fontAntialiasing WRITE setFontAntialiasing NOTIFY settingsChanged)
    Q_PROPERTY(QString fontRgbaOrder READ fontRgbaOrder WRITE setFontRgbaOrder NOTIFY settingsChanged)
    Q_PROPERTY(double textScalingFactor READ textScalingFactor WRITE setTextScalingFactor NOTIFY settingsChanged)
    Q_PROPERTY(QStringList themes READ themes NOTIFY settingsChanged)
    Q_PROPERTY(QStringList themeIds READ themeIds NOTIFY settingsChanged)
    Q_PROPERTY(QStringList iconThemes READ iconThemes NOTIFY settingsChanged)
    Q_PROPERTY(QStringList iconThemeIds READ iconThemeIds NOTIFY settingsChanged)
    Q_PROPERTY(QStringList cursorThemes READ cursorThemes NOTIFY settingsChanged)
    Q_PROPERTY(QStringList cursorThemeIds READ cursorThemeIds NOTIFY settingsChanged)

public:
    explicit GtkSettingsInfo(QObject *parent = nullptr);

    QString theme() const;
    QString iconTheme() const;
    QString cursorTheme() const;
    QString font() const;
    int cursorSize() const;
    QString colorScheme() const;
    bool eventSounds() const;
    bool inputFeedbackSounds() const;
    QString fontHinting() const;
    QString fontAntialiasing() const;
    QString fontRgbaOrder() const;
    double textScalingFactor() const;
    QStringList themes() const;
    QStringList themeIds() const;
    QStringList iconThemes() const;
    QStringList iconThemeIds() const;
    QStringList cursorThemes() const;
    QStringList cursorThemeIds() const;

    void setTheme(const QString &value);
    void setIconTheme(const QString &value);
    void setCursorTheme(const QString &value);
    void setFont(const QString &value);
    void setCursorSize(int value);
    void setColorScheme(const QString &value);
    void setEventSounds(bool value);
    void setInputFeedbackSounds(bool value);
    void setFontHinting(const QString &value);
    void setFontAntialiasing(const QString &value);
    void setFontRgbaOrder(const QString &value);
    void setTextScalingFactor(double value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();
    Q_INVOKABLE QString fontToString(const QFont &font) const;
    Q_INVOKABLE QVariantList gtkThemePreviews(const QString &theme) const;

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    void setChanged();
    void scanThemes();
    void scanIconsAndCursors();

    QString m_theme;
    QString m_iconTheme;
    QString m_cursorTheme;
    QString m_font;
    int m_cursorSize = 24;
    QString m_colorScheme = QStringLiteral("default");
    bool m_eventSounds = true;
    bool m_inputFeedbackSounds = false;
    QString m_fontHinting = QStringLiteral("medium");
    QString m_fontAntialiasing = QStringLiteral("grayscale");
    QString m_fontRgbaOrder = QStringLiteral("rgb");
    double m_textScalingFactor = 1.0;
    QStringList m_themes;
    QStringList m_themeIds;
    QStringList m_iconThemes;
    QStringList m_iconThemeIds;
    QStringList m_cursorThemes;
    QStringList m_cursorThemeIds;
};
