// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QDir>
#include <QQmlExtensionPlugin>

class MauiSettingsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override;

private:
    QUrl componentUrl(const QString &fileName) const;

    void initializeEngine(QQmlEngine *engine, const char *uri) override;
    
    QString resolveFileUrl(const QString &filePath) const
    {
        return baseUrl().toString() + QLatin1Char('/') + filePath;
    }
};
