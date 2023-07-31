// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "settings_plugin.h"

#include <QQmlEngine>
#include <QResource>
#include <QUrl>

void MauiSettingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.maui.settings.lib"));

    qmlRegisterType(componentUrl(QStringLiteral("SettingsPage.qml")), uri, 1, 0, ("SettingsPage"));
    qmlRegisterType(componentUrl(QStringLiteral("SettingsLayout.qml")), uri, 1, 0, ("SettingsLayout"));
    qmlRegisterType(componentUrl(QStringLiteral("GraphicButton.qml")), uri, 1, 0, ("GraphicButton"));
}

QUrl MauiSettingsPlugin::componentUrl(const QString &fileName) const
{
    return QUrl(resolveFileUrl(fileName));
}

void MauiSettingsPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri);
}
