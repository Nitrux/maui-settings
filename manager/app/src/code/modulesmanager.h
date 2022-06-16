#pragma once

#include <QObject>
#include "modulesmodel.h"

class ModulesManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ModulesModel * model READ model CONSTANT FINAL)
    Q_PROPERTY(bool serverRunning READ serverRunning NOTIFY serverRunningChanged)
public:
    explicit ModulesManager(QObject *parent = nullptr);

    ModulesModel * model() const;

    bool serverRunning() const;

public slots:
    void startServer();
    AbstractModule *sourceFor(const QString &moduleName);

private:
    ModulesModel * m_model;

    bool m_serverRunning = false;

signals:

    void serverRunningChanged(bool serverRunning);
};

