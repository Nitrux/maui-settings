#pragma once

#include <QObject>
#include <QString>

class Theme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int styleType READ styleType WRITE setStyleType NOTIFY styleTypeChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(QString iconTheme READ iconTheme WRITE setIconTheme NOTIFY iconThemeChanged)
    Q_PROPERTY(QString windowControlsTheme READ windowControlsTheme WRITE setWindowControlsTheme NOTIFY windowControlsThemeChanged)

public:
    explicit Theme(QObject * parent = nullptr);

    int styleType() const;
    void setStyleType(int newStyleType);

    const QString &accentColor() const;
    void setAccentColor(const QString &newAccentColor);

    const QString &iconTheme() const;
    void setIconTheme(const QString &newIconTheme);

    const QString &windowControlsTheme() const;
    void setWindowControlsTheme(const QString &newWindowControlsTheme);

signals:
    void styleTypeChanged(int styleStype);
    void accentColorChanged(QString accentColor);

    void iconThemeChanged(QString iconTheme);

    void windowControlsThemeChanged(QString windowControlsTheme);

private:
    int m_styleType;
    QString m_accentColor;
    QString m_iconTheme;
    QString m_windowControlsTheme;
};
