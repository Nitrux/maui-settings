/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Kevin Ottens <ervin+bluesystems@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <qpa/qplatformthemeplugin.h>

#include "mauiplatformtheme.h"

class MauiPlatformThemePlugin : public QPlatformThemePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPA.QPlatformThemeFactoryInterface.5.1" FILE "mauiplatformtheme.json")
public:
    MauiPlatformThemePlugin(QObject *parent = nullptr)
        : QPlatformThemePlugin(parent)
    {
    }

    QPlatformTheme *create(const QString &key, const QStringList &paramList) override
    {
        Q_UNUSED(key)
        Q_UNUSED(paramList)
        return new MauiPlatformTheme;
    }
};

#include "main.moc"
