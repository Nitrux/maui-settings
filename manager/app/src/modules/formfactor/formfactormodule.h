#pragma once

#include "code/abstractmodule.h"
#include <MauiMan4/formfactormanager.h>

class FormFactorModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(MauiMan::FormFactorManager* manager READ manager CONSTANT FINAL)
public:
    FormFactorModule(QObject * parent = nullptr);

    MauiMan::FormFactorManager* manager();

private:
    MauiMan::FormFactorManager *m_manager;
};

