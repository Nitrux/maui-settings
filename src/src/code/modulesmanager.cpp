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
    QQmlEngine engine;

    BackgroundModule backgroundModule;
    qDebug() << "IMPORT PATH" <<  engine.importPathList() << backgroundModule.name() << backgroundModule.keywords();

}

ModulesModel *ModulesManager::model() const
{
    return m_model;
}
