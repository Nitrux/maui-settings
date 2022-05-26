#include "thememanager.h"

#include "code/settingsstore.h"
#include "code/mauimanutils.h"

#include <QDebug>

using namespace MauiMan;
ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
  ,m_interface("org.mauiman.Manager",
               "/Theme",
               "org.mauiman.Theme",
               QDBusConnection::sessionBus(), this)
  ,m_settings(new MauiMan::SettingsStore(this))
{
    qDebug( " INIT THEME MANAGER");
    m_settings->beginModule("Theme");
    m_accentColor = m_settings->load("AccentColor", m_accentColor).toString();
    m_styleType = m_settings->load("StyleType", m_styleType).toInt();
    m_iconTheme = m_settings->load("IconTheme", m_iconTheme).toString();
    m_windowControlsTheme = m_settings->load("WindowControlsTheme", m_windowControlsTheme).toString();

    auto server = new MauiManUtils(this);
    if(server->serverRunning())
    {
        this->setConnections();
    }

    connect(server, &MauiManUtils::serverRunningChanged, [this](bool state)
    {
        if(state)
        {
            this->setConnections();
        }
    });
}


void ThemeManager::sync(const QString &key, const QVariant &value)
{
    if (m_interface.isValid())
    {
        m_interface.call(key, value);
    }
}

void ThemeManager::setConnections()
{
    m_interface.disconnect();
    if (m_interface.isValid())
    {
        connect(&m_interface, SIGNAL(accentColorChanged(QString)), this, SLOT(onAccentColorChanged(QString)));
        connect(&m_interface, SIGNAL(iconThemeChanged(QString)), this, SLOT(onIconThemeChanged(QString)));
        connect(&m_interface, SIGNAL(windowControlsThemeChanged(QString)), this, SLOT(onWindowControlsThemeChanged(bool)));
        connect(&m_interface, SIGNAL(styleTypeChanged(int)), this, SLOT(onStyleTypeChanged(int)));
    }
}

int ThemeManager::styleType() const
{
    return m_styleType;
}

void ThemeManager::setStyleType(int newStyleType)
{
    if (m_styleType == newStyleType)
        return;

    m_styleType = newStyleType;
    m_settings->save("StyleType", m_styleType);
    sync("setStyleType", newStyleType);
    emit styleTypeChanged(m_styleType);

}

const QString &ThemeManager::accentColor() const
{
    return m_accentColor;
}

void ThemeManager::setAccentColor(const QString &newAccentColor)
{
    if (m_accentColor == newAccentColor)
        return;

    qDebug() << "Setting accent color" << m_accentColor;

    m_accentColor = newAccentColor;
    m_settings->save("AccentColor", m_accentColor);
    sync("setAccentColor", m_accentColor);
    emit accentColorChanged(m_accentColor);
}

const QString &ThemeManager::iconTheme() const
{
    return m_iconTheme;
}

void ThemeManager::setIconTheme(const QString &newIconTheme)
{
    if (m_iconTheme == newIconTheme)
        return;

    m_iconTheme = newIconTheme;
    m_settings->save("IconTheme", m_iconTheme);
    sync("setIconTheme", m_iconTheme);
    emit iconThemeChanged(m_iconTheme);
}

const QString &ThemeManager::windowControlsTheme() const
{
    return m_windowControlsTheme;
}

void ThemeManager::setWindowControlsTheme(const QString &newWindowControlsTheme)
{
    if (m_windowControlsTheme == newWindowControlsTheme)
        return;

    m_windowControlsTheme = newWindowControlsTheme;
    m_settings->save("WindowControlsTheme", m_windowControlsTheme);
    sync("setWindowControlsTheme", m_windowControlsTheme);
    emit windowControlsThemeChanged(m_windowControlsTheme);
}

void ThemeManager::onStyleTypeChanged(const int &newStyleType)
{
    if (m_styleType == newStyleType)
        return;
    m_styleType = newStyleType;
    emit styleTypeChanged(m_styleType);
}

void ThemeManager::onAccentColorChanged(const QString &newAccentColor)
{
    if (m_accentColor == newAccentColor)
        return;
    m_accentColor = newAccentColor;
    emit accentColorChanged(m_accentColor);
}

void ThemeManager::onWindowControlsThemeChanged(const QString &newWindowControlsTheme)
{
    if (m_windowControlsTheme == newWindowControlsTheme)
        return;
    m_windowControlsTheme = newWindowControlsTheme;
    emit windowControlsThemeChanged(m_windowControlsTheme);
}

void ThemeManager::onIconThemeChanged(const QString &newIconTheme)
{
    if (m_iconTheme == newIconTheme)
        return;
    m_iconTheme = newIconTheme;
    emit iconThemeChanged(m_iconTheme);
}
