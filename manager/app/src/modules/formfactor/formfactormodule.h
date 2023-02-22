#pragma once

#include "code/abstractmodule.h"
namespace MauiMan
{
class FormFactorManager;
}

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

