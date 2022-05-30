// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "settings_plugin.h"

#include <QQmlEngine>
#include <QResource>

void MauiSettingsPlugin::registerTypes(const char *uri)
{

    qmlRegisterType(resolveFileUrl(QStringLiteral("SettingsPage.qml")), uri, 1, 0, "SettingsPage");
    qmlRegisterType(resolveFileUrl(QStringLiteral("SettingsLayout.qml")), uri, 1, 0, "SettingsLayout");
    qmlRegisterType(resolveFileUrl(QStringLiteral("GraphicButton.qml")), uri, 1, 0, "GraphicButton");
}

void MauiSettingsPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri);
}
