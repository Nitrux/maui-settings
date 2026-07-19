#pragma once

#include <QObject>
#include <QColor>
#include <QFont>
#include <QString>
#include <QStringList>

class KdeGlobalsInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(QString colorScheme READ colorScheme WRITE setColorScheme NOTIFY settingsChanged)
    Q_PROPERTY(QString iconTheme READ iconTheme WRITE setIconTheme NOTIFY settingsChanged)
    Q_PROPERTY(QString defaultFont READ defaultFont WRITE setDefaultFont NOTIFY settingsChanged)
    Q_PROPERTY(QString smallFont READ smallFont WRITE setSmallFont NOTIFY settingsChanged)
    Q_PROPERTY(QString monospaceFont READ monospaceFont WRITE setMonospaceFont NOTIFY settingsChanged)
    Q_PROPERTY(QStringList colorSchemes READ colorSchemes NOTIFY settingsChanged)
    Q_PROPERTY(QStringList iconThemes READ iconThemes NOTIFY settingsChanged)

public:
    explicit KdeGlobalsInfo(QObject *parent = nullptr);

    QString configPath() const;
    QString colorScheme() const;
    QString iconTheme() const;
    QString defaultFont() const;
    QString smallFont() const;
    QString monospaceFont() const;
    QStringList colorSchemes() const;
    QStringList iconThemes() const;

    void setColorScheme(const QString &value);
    void setIconTheme(const QString &value);
    void setDefaultFont(const QString &value);
    void setSmallFont(const QString &value);
    void setMonospaceFont(const QString &value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();

    Q_INVOKABLE QFont fontFromString(const QString &value) const;
    Q_INVOKABLE QString fontToString(const QFont &font) const;
    Q_INVOKABLE QString fontLabel(const QString &value) const;
    Q_INVOKABLE QColor colorSchemePreviewColor(const QString &scheme) const;

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    void setChanged();
    QStringList scanColorSchemes() const;
    QStringList scanIconThemes() const;
    QString colorSchemeFilePath(const QString &scheme) const;

private:
    QString m_configPath;
    QString m_colorScheme;
    QString m_iconTheme;
    QString m_defaultFont;
    QString m_smallFont;
    QString m_monospaceFont;
    QStringList m_colorSchemes;
    QStringList m_iconThemes;
};
