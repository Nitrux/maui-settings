// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QDate>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <MauiKit/Core/mauiapp.h>

#include <KAboutData>
#include <KI18n/KLocalizedString>

#include "../mauisettings_version.h"

#include "src/code/modulesmanager.h"
#include "src/code/modulesmodel.h"

#define MAUISETTINGS_URI "org.maui.settings"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    QApplication app(argc, argv);

    app.setOrganizationName(QStringLiteral("Maui"));
    app.setWindowIcon(QIcon(":/nxmanager.svg"));

    MauiApp::instance()->setIconName("qrc:/nxmanager.svg");

    KLocalizedString::setApplicationDomain("settings");

    KAboutData about(QStringLiteral("manager"), i18n("Maui Settings"), MAUISETTINGS_VERSION_STRING, i18n("Maui Settings Manager."),
                     KAboutLicense::LGPL_V3, i18n("© 2019-%1 Maui Development Team", QString::number(QDate::currentDate().year())), QString(GIT_BRANCH) + "/" + QString(GIT_COMMIT_HASH));

    about.addAuthor(i18n("Camilo Higuita"), i18n("Developer"), QStringLiteral("milo.h@aol.com"));
    about.setHomepage("https://nxos.org");
    about.setProductName("maui/manager");
    about.setBugAddress("https://github.com/nitrux/nxmanager");
    about.setOrganizationDomain(MAUISETTINGS_URI);
    about.setProgramLogo(app.windowIcon());

    KAboutData::setApplicationData(about);

    ModulesManager manager;
    QCommandLineParser parser;

    if(about.setupCommandLine(&parser))
    {
        qDebug() << "App data was set correctly";
    }

    parser.setApplicationDescription(about.shortDescription());
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption moduleOption(QStringList() << "m" << "module",
                                    QCoreApplication::translate("main", "open the manager at the given module name"),
                                    QCoreApplication::translate("main", "module"));
    parser.addOption(moduleOption);

    parser.process(app);
    about.processCommandLine(&parser);

    if (parser.isSet(moduleOption))
    {

    }

    qmlRegisterAnonymousType<ModulesModel>(MAUISETTINGS_URI, 1);
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("ModulesManager", &manager);
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
