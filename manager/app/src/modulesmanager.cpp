#include "modulesmanager.h"

#include <QDir>
#include <QDebug>
#include <QQmlEngine>
#include <QVector>
#include "code/abstractmodule.h"

//modules
#include "modules/background/backgroundmodule.h"
#include "modules/theme/thememodule.h"
#include "modules/about/aboutmodule.h"
#include "modules/screen/screenmodule.h"
#include "modules/audio/audiomodule.h"
#include "modules/shell/shellmodule.h"
#include "modules/network/networkmodule.h"
#include "modules/bluetooth/bluetoothmodule.h"
#include "modules/formfactor/formfactormodule.h"
#include "modules/accessibility/accessibilitymodule.h"
#include "modules/screenshot/screenshotmodule.h"

#include <MauiMan/mauimanutils.h>
#include <CaskServer/serverutils.h>

ModulesManager::ModulesManager(QObject *parent) : QObject(parent)
  ,m_model(new ModulesModel(this))
{
    //    QString path = MAUIMAN_MODULES_IMPORT_PATH;
    //    QQmlEngine engine;
    auto server = new MauiManUtils(this);
    m_serverRunning = server->serverRunning();

    connect(server, &MauiManUtils::serverRunningChanged, [this](bool state)
    {
        m_serverRunning = state;
        emit serverRunningChanged(m_serverRunning);
    });

    auto caskServer = new ServerUtils(this);
    m_caskServerRunning = caskServer->serverRunning();

    connect(caskServer, &ServerUtils::serverRunningChanged, [this](bool state)
    {
        m_caskServerRunning = state;
        emit caskServerRunningChanged(m_caskServerRunning);
    });

//general
    m_model->appendModule(new AboutModule);

    //apperance
    m_model->appendModule(new BackgroundModule);
    m_model->appendModule(new ThemeModule);

    //network
    m_model->appendModule(new BluetoothModule);
    m_model->appendModule(new NetworkModule);

    //system
    m_model->appendModule(new ScreenModule);

    m_model->appendModule(new AudioModule);
    m_model->appendModule(new FormFactorModule);
    m_model->appendModule(new AccessibilityModule);

    //shell
    m_model->appendModule(new ShellModule);
    m_model->appendModule(new ScreenshotModule);
}

ModulesModel *ModulesManager::model() const
{
    return m_model;
}

bool ModulesManager::serverRunning() const
{
    return m_serverRunning;
}

bool ModulesManager::isMauiSession() const
{
    return MauiManUtils::isMauiSession();
}

QString ModulesManager::currentDesktopSession() const
{
    return MauiManUtils::currentDesktopSession();
}

bool ModulesManager::caskServerRunning() const
{
    return m_caskServerRunning;
}

void ModulesManager::startServer()
{
    MauiManUtils::startServer();
}

AbstractModule* ModulesManager::sourceFor(const QString &moduleId)
{
    for(const auto &module : m_model->modules())
    {
        if(module->id() == moduleId)
        {
            return module;
        }
    }

    return nullptr;
}
