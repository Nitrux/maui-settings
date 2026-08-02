#pragma once

#include <QObject>
#include <QColor>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class KdeGlobalsInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(QString colorScheme READ colorScheme WRITE setColorScheme NOTIFY settingsChanged)
    Q_PROPERTY(QString iconTheme READ iconTheme WRITE setIconTheme NOTIFY settingsChanged)
    Q_PROPERTY(QString cursorTheme READ cursorTheme WRITE setCursorTheme NOTIFY settingsChanged)
    Q_PROPERTY(QString defaultFont READ defaultFont WRITE setDefaultFont NOTIFY settingsChanged)
    Q_PROPERTY(QString smallFont READ smallFont WRITE setSmallFont NOTIFY settingsChanged)
    Q_PROPERTY(QString monospaceFont READ monospaceFont WRITE setMonospaceFont NOTIFY settingsChanged)
    Q_PROPERTY(bool singleClick READ singleClick WRITE setSingleClick NOTIFY settingsChanged)
    Q_PROPERTY(QStringList colorSchemes READ colorSchemes NOTIFY settingsChanged)
    Q_PROPERTY(QStringList iconThemes READ iconThemes NOTIFY settingsChanged)
    Q_PROPERTY(QStringList iconThemeIds READ iconThemeIds NOTIFY settingsChanged)
    Q_PROPERTY(QStringList cursorThemes READ cursorThemes NOTIFY settingsChanged)
    Q_PROPERTY(QStringList cursorThemeIds READ cursorThemeIds NOTIFY settingsChanged)

public:
    explicit KdeGlobalsInfo(QObject *parent = nullptr);

    QString configPath() const;
    QString colorScheme() const;
    QString iconTheme() const;
    QString cursorTheme() const;
    QString defaultFont() const;
    QString smallFont() const;
    QString monospaceFont() const;
    bool singleClick() const;
    QStringList colorSchemes() const;
    QStringList iconThemes() const;
    QStringList iconThemeIds() const;
    QStringList cursorThemes() const;
    QStringList cursorThemeIds() const;

    void setColorScheme(const QString &value);
    void setIconTheme(const QString &value);
    void setCursorTheme(const QString &value);
    void setDefaultFont(const QString &value);
    void setSmallFont(const QString &value);
    void setMonospaceFont(const QString &value);
    void setSingleClick(bool value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();

    Q_INVOKABLE QFont fontFromString(const QString &value) const;
    Q_INVOKABLE QString fontToString(const QFont &font) const;
    Q_INVOKABLE QString fontLabel(const QString &value) const;
    Q_INVOKABLE QVariantMap colorSchemePreview(const QString &scheme) const;
    Q_INVOKABLE QString colorSchemeFilePath(const QString &scheme) const;
    Q_INVOKABLE QVariantList iconThemePreviewIcons(const QString &theme) const;
    Q_INVOKABLE QVariantList cursorThemePreviewImages(const QString &theme) const;

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    void setChanged();
    QStringList scanColorSchemes() const;
    QVariantList scanIconThemes() const;
    QVariantList scanCursorThemes() const;

private:
    QString m_configPath;
    QString m_inputConfigPath;
    QString m_colorScheme;
    QString m_iconTheme;
    QString m_cursorTheme;
    QString m_defaultFont;
    QString m_smallFont;
    QString m_monospaceFont;
    bool m_singleClick = true;
    QStringList m_colorSchemes;
    QStringList m_iconThemes;
    QStringList m_iconThemeIds;
    QStringList m_cursorThemes;
    QStringList m_cursorThemeIds;
};
