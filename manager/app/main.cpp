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
#include <QPair>

#include <MauiKit/Core/mauiapp.h>

#include <KAboutData>
#include <KI18n/KLocalizedString>

#include "../mauisettings_version.h"

#include "src/modulesmanager.h"
#include "src/modulesmodel.h"

#include "src/server/server.h"

#define MAUISETTINGS_URI "org.maui.settings"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    QApplication app(argc, argv);

    app.setOrganizationName(QStringLiteral("Maui"));
    app.setWindowIcon(QIcon(":/nxmanager.svg"));

    KLocalizedString::setApplicationDomain("maui-settings");

    KAboutData about(QStringLiteral("settings"), i18n("Maui Settings"), MAUISETTINGS_VERSION_STRING, i18n("Maui Settings Manager."),
                     KAboutLicense::LGPL_V3, i18n("© 2022-%1 Maui Development Team", QString::number(QDate::currentDate().year())), QString(GIT_BRANCH) + "/" + QString(GIT_COMMIT_HASH));

    about.addAuthor(i18n("Camilo Higuita"), i18n("Developer"), QStringLiteral("milo.h@aol.com"));
    about.setHomepage("https://nxos.org");
    about.setProductName("maui/settings");
    about.setBugAddress("https://github.com/nitrux/nxmanager");
    about.setOrganizationDomain(MAUISETTINGS_URI);
    about.setProgramLogo(app.windowIcon());

    KAboutData::setApplicationData(about);
    MauiApp::instance()->setIconName("qrc:/nxmanager.svg");

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

    QPair<QString, QVariant> arguments;

    if (parser.isSet(moduleOption))
    {
        arguments.first = parser.value(moduleOption);
    }else
    {
        arguments.first = "about";
    }

    if (AppInstance::attachToExistingInstance(arguments.first))
    {
        // Successfully attached to existing instance of Nota
        return 0;
    }

    AppInstance::registerService();

    auto server =  std::make_unique<Server>();

    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url, &server, &arguments](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);

        server->setQmlObject(obj);

        server->openModule(arguments.first);
    }, Qt::QueuedConnection);

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    qmlRegisterAnonymousType<ModulesModel>(MAUISETTINGS_URI, 1);

    engine.rootContext()->setContextProperty("ModulesManager", &manager);

    engine.load(url);

    return app.exec();
}
