// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QQmlEngine>
#include <QResource>

#include "touchegg_plugin.h"

void ToucheggPlugin::registerTypes(const char *uri)
{
    qmlRegisterType(resolveFileUrl(QStringLiteral("Touchegg.qml")), uri, 1, 0, "Touchegg");
}

