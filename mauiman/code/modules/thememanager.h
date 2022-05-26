#pragma once

#include <QObject>
#include <QString>
#include <QDBusInterface>
#include "mauiman_export.h"


namespace MauiMan
{
class SettingsStore;
}

/**
 * @brief The ThemeManager class
 * Helpfull for third parties to connect to property cahnges form the Background module setting changes.
 */
namespace MauiMan
{
class MAUIMAN_EXPORT ThemeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int styleType READ styleType WRITE setStyleType NOTIFY styleTypeChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(QString iconTheme READ iconTheme WRITE setIconTheme NOTIFY iconThemeChanged)
    Q_PROPERTY(QString windowControlsTheme READ windowControlsTheme WRITE setWindowControlsTheme NOTIFY windowControlsThemeChanged)

public:
    explicit ThemeManager(QObject * parent = nullptr);

    enum StyleType
    {
        Light = 0,
        Dark = 1,
        Adaptive = 2,
        Auto = 3
    };

    int styleType() const;
    void setStyleType(int newStyleType);

    const QString &accentColor() const;
    void setAccentColor(const QString &newAccentColor);

    const QString &iconTheme() const;
    void setIconTheme(const QString &newIconTheme);

    const QString &windowControlsTheme() const;
    void setWindowControlsTheme(const QString &newWindowControlsTheme);

private slots:
    void onStyleTypeChanged(const int &newStyleType);
    void onAccentColorChanged(const QString &newAccentColor);
    void onWindowControlsThemeChanged(const QString &newWindowControlsTheme);
    void onIconThemeChanged(const QString &newIconTheme);

signals:

    void styleTypeChanged(int styleType);

    void accentColorChanged(QString accentColor);

    void iconThemeChanged(QString iconTheme);

    void windowControlsThemeChanged(QString windowControlsTheme);

private:
    QDBusInterface m_interface;
    MauiMan::SettingsStore *m_settings;
    int m_styleType;
    QString m_accentColor;
    QString m_iconTheme;
    QString m_windowControlsTheme;
    void sync(const QString &key, const QVariant &value);
   void setConnections();
};
}

