#include "modulesmanager.h"

#include <QDir>
#include <QDebug>
#include <QQmlEngine>

//modules
#include "modules/background/src/code/backgroundmodule.h"

ModulesManager::ModulesManager(QObject *parent) : QObject(parent)
  ,m_model(new ModulesModel(this))
{
//    QString path = MAUIMAN_MODULES_IMPORT_PATH;
//    QQmlEngine engine;

    m_model->appendModule(new BackgroundModule);
}

ModulesModel *ModulesManager::model() const
{
    return m_model;
}
