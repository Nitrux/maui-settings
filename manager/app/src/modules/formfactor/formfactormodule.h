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


    // AbstractModule interface
public:
    QString qmlSource() const override final;
    QString iconName() const override final;
    QString name() const override final;
    QString category() const override final;
    QStringList keywords() const override final;

    QString description() const override final;

    MauiMan::FormFactorManager* manager();

private:
    MauiMan::FormFactorManager *m_manager;
};

