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
#include "modules/inputdevices/inputdevicesmodule.h"

#include <MauiMan4/mauimanutils.h>
#include <CaskServer/serverutils.h>

ModulesManager::ModulesManager(QObject *parent) : QObject(parent)
  ,m_model(new ModulesProxyModel(this))
{
    //    QString path = MAUIMAN_MODULES_IMPORT_PATH;
    //    QQmlEngine engine;
    auto server = new MauiManUtils(this);
    m_serverRunning = server->serverRunning();

    connect(server, &MauiManUtils::serverRunningChanged, [this](bool state)
    {
        m_serverRunning = state;
        Q_EMIT serverRunningChanged(m_serverRunning);
    });

    auto caskServer = new ServerUtils(this);
    m_caskServerRunning = caskServer->serverRunning();

    connect(caskServer, &ServerUtils::serverRunningChanged, [this](bool state)
    {
        m_caskServerRunning = state;
        Q_EMIT caskServerRunningChanged(m_caskServerRunning);
    });

    //general
    m_model->model()->appendModule(new AboutModule);

    //apperance
    m_model->model()->appendModule(new BackgroundModule);
    m_model->model()->appendModule(new ThemeModule);
    
    //network
    m_model->model()->appendModule(new BluetoothModule);
    m_model->model()->appendModule(new NetworkModule);
    
    //system
    m_model->model()->appendModule(new ScreenModule);
    
    m_model->model()->appendModule(new AudioModule);
    m_model->model()->appendModule(new InputDevicesModule);
    m_model->model()->appendModule(new FormFactorModule);
    m_model->model()->appendModule(new AccessibilityModule);
    
    //shell
    // m_model->model()->appendModule(new ShellModule);
    // m_model->model()->appendModule(new ScreenshotModule);
}

ModulesProxyModel *ModulesManager::model() const
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

void ModulesManager::startCaskServer()
{
    ServerUtils::startServer();
}

void ModulesManager::startMauiManServer()
{
    MauiManUtils::startServer();
}

AbstractModule* ModulesManager::sourceFor(const QString &moduleId)
{
    auto modules= m_model->model()->modules();
    for(const auto &module : modules)
    {
        qDebug() << "lookin for modules" << moduleId;
        if(module->id() == moduleId)
        {
            return module;
        }
    }

    return nullptr;
}
