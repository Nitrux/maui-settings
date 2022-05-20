#pragma once

#include <QObject>
#include "modulesmodel.h"

class ModulesManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ModulesModel * model READ model CONSTANT FINAL)

public:
    explicit ModulesManager(QObject *parent = nullptr);

    ModulesModel * model() const;

public slots:


private:
    ModulesModel * m_model;

signals:

};

