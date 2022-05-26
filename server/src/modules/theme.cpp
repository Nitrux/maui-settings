#include "theme.h"
#include "themeadaptor.h"
#include <QDBusInterface>

#include "code/settingsstore.h"

#include <QDebug>

Theme::Theme(QObject *parent) : QObject(parent)
{
    qDebug( " INIT THEME MANAGER");
    new ThemeAdaptor(this);
    if(!QDBusConnection::sessionBus().registerObject(QStringLiteral("/Theme"), this))
    {
        qDebug() << "FAILED TO REGISTER THEME DBUS OBJECT";
        return;
    }
MauiMan::SettingsStore settings;
    settings.beginModule("Theme");
    m_accentColor = settings.load("AccentColor", m_accentColor).toString();
    m_iconTheme = settings.load("IconTheme", m_iconTheme).toString();
    m_windowControlsTheme = settings.load("WindowControlsTheme", m_windowControlsTheme).toString();
    m_styleType = settings.load("StyleType", m_styleType).toInt();
    settings.endModule();
}

int Theme::styleType() const
{
    return m_styleType;
}

void Theme::setStyleType(int newStyleType)
{
    if (m_styleType == newStyleType)
        return;
    m_styleType = newStyleType;
    emit styleTypeChanged(m_styleType);
}

const QString &Theme::accentColor() const
{
    return m_accentColor;
}

void Theme::setAccentColor(const QString &newAccentColor)
{
    if (m_accentColor == newAccentColor)
        return;
    m_accentColor = newAccentColor;
    emit accentColorChanged(m_accentColor);
}

const QString &Theme::iconTheme() const
{
    return m_iconTheme;
}

void Theme::setIconTheme(const QString &newIconTheme)
{
    if (m_iconTheme == newIconTheme)
        return;
    m_iconTheme = newIconTheme;
    emit iconThemeChanged(m_iconTheme);
}

const QString &Theme::windowControlsTheme() const
{
    return m_windowControlsTheme;
}

void Theme::setWindowControlsTheme(const QString &newWindowControlsTheme)
{
    if (m_windowControlsTheme == newWindowControlsTheme)
        return;
    m_windowControlsTheme = newWindowControlsTheme;
    emit windowControlsThemeChanged(m_windowControlsTheme);
}
