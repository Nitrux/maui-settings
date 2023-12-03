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
#include <memory>
#include <MauiKit4/Core/mauiapp.h>

#include <KAboutData>
#include <KLocalizedString>

#include "../mauisettings_version.h"

#include "src/modulesmanager.h"
#include "src/modulesmodel.h"

#include "src/server/server.h"

#include "code/abstractmodule.h"


#define MAUISETTINGS_URI "org.maui.settings"

#undef QT_NO_CAST_TO_ASCII
#undef QT_NO_CAST_FROM_ASCII

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName(QStringLiteral("Maui"));
    app.setWindowIcon(QIcon(QStringLiteral(":/mauisettings.svg")));

    KLocalizedString::setApplicationDomain("maui-settings");

    KAboutData about(QStringLiteral("mauisettings"),
                     i18n("Maui Settings"),
                     MAUISETTINGS_VERSION_STRING,
                     i18n("Maui Settings Manager."),
                     KAboutLicense::LGPL_V3,
                     i18n("© 2022-%1 Maui Development Team", QString::number(QDate::currentDate().year())),
                     QString(GIT_BRANCH) + QStringLiteral("/") + QString(GIT_COMMIT_HASH));

    about.addAuthor(QStringLiteral("Camilo Higuita"), i18n("Developer"), QStringLiteral("milo.h@aol.com"));
    about.setHomepage(QStringLiteral("https://nxos.org"));
    about.setProductName("maui/settings");
    about.setBugAddress("https://github.com/nitrux/maui-settings");
    about.setOrganizationDomain(MAUISETTINGS_URI);
    about.setProgramLogo(app.windowIcon());

    KAboutData::setApplicationData(about);
    MauiApp::instance()->setIconName(QStringLiteral("qrc:/mauisettings.svg"));

    std::unique_ptr<ModulesManager> manager = std::make_unique<ModulesManager>();
    QCommandLineParser parser;

    if(about.setupCommandLine(&parser))
    {
        qDebug() << "App data was set correctly";
    }

    parser.setApplicationDescription(about.shortDescription());
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption moduleOption(QStringList() << QStringLiteral("m") << QStringLiteral("module"),
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
        arguments.first = QStringLiteral("about");
    }

    if (AppInstance::attachToExistingInstance(arguments.first))
    {
        // Successfully attached to existing instance of Nota
        return 0;
    }

    AppInstance::registerService();

    auto server =  std::make_unique<Server>();

    QQmlApplicationEngine engine;

    const QUrl url(u"qrc:/MauiSettingsModule/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url, &server, &arguments](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);

        server->setQmlObject(obj);

        server->openModule(arguments.first);
    }, Qt::QueuedConnection);

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    qmlRegisterAnonymousType<ModulesModel>(MAUISETTINGS_URI, 1);
    qmlRegisterAnonymousType<AbstractModule>(MAUISETTINGS_URI, 1);

    engine.rootContext()->setContextProperty(QStringLiteral("ModulesManager"), manager.get());

    engine.load(url);

    return app.exec();
}
