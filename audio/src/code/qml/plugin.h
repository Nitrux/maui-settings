/*
    SPDX-FileCopyrightText: 2014-2015 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once
#include <QQmlExtensionPlugin>

class AudioPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes(const char *uri) override;

private:
    QUrl componentUrl(const QString &fileName) const;

    QString resolveFileUrl(const QString &filePath) const
    {
#ifdef QUICK_COMPILER
        return QStringLiteral("qrc:/mauicore/audio/") + filePath;
#else
        return baseUrl().toString() + QLatin1Char('/') + filePath;
#endif
    }
};

