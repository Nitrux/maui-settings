// Copyright 2026 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDate>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include <QUrl>

#include <KLocalizedContext>

#include <KAboutData>
#include <KLocalizedString>

#include <MauiKit4/Core/mauiapp.h>
#include <MauiMan4/thememanager.h>

#include "controllers/aboutinfo.h"
#include "controllers/backgroundinfo.h"
#include "controllers/kdeglobalsinfo.h"
#include "controllers/marinainfo.h"
#include "controllers/nudgeosdinfo.h"
#include "controllers/valenzinfo.h"

static constexpr auto SETTINGS_URI = "org.maui.settings";

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    app.setOrganizationName(QStringLiteral("Maui"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-theme")));

    KLocalizedString::setApplicationDomain("maui-settings");

    KAboutData about(QStringLiteral("maui-settings"),
                     i18n("Workspace Settings"),
                     QStringLiteral("0.0.1"),
                     i18n("A simple settings shell built with MauiKit."),
                     KAboutLicense::GPL_V3,
                     i18n("© %1 Made by Nitrux | Built with MauiKit", QString::number(QDate::currentDate().year())),
                     QStringLiteral(GIT_BRANCH) + QStringLiteral("/") + QStringLiteral(GIT_COMMIT_HASH));

    about.addAuthor(QStringLiteral("Uri Herrera"), i18n("Developer"), QStringLiteral("uri_herrera@nxos.org"));
    about.setHomepage(QStringLiteral("https://nxos.org"));
    about.setOrganizationDomain(QByteArrayLiteral("org.maui.settings"));
    about.setProductName(QByteArrayLiteral("nitrux/maui-settings"));
    about.setProgramLogo(app.windowIcon());
    about.setDesktopFileName(QStringLiteral("org.maui.settings"));
    KAboutData::setApplicationData(about);

    MauiApp::instance()->setIconName(QStringLiteral("preferences-desktop-theme"));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    AboutInfo aboutInfo;
    engine.rootContext()->setContextProperty(QStringLiteral("aboutInfo"), &aboutInfo);

    BackgroundInfo backgroundInfo;
    engine.rootContext()->setContextProperty(QStringLiteral("backgroundInfo"), &backgroundInfo);

    MauiMan::ThemeManager themeInfo;
    engine.rootContext()->setContextProperty(QStringLiteral("themeInfo"), &themeInfo);

    KdeGlobalsInfo kdeGlobalsInfo;
    engine.rootContext()->setContextProperty(QStringLiteral("kdeGlobalsInfo"), &kdeGlobalsInfo);

    MarinaInfo marinaInfo;
    engine.rootContext()->setContextProperty(QStringLiteral("marinaInfo"), &marinaInfo);

    NudgeOsdInfo nudgeOsdInfo;
    engine.rootContext()->setContextProperty(QStringLiteral("nudgeOsdInfo"), &nudgeOsdInfo);

    ValenzInfo valenzInfo;
    engine.rootContext()->setContextProperty(QStringLiteral("valenzInfo"), &valenzInfo);

    const QUrl url(QStringLiteral("qrc:/app/maui/settings/src/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}
