#pragma once

#include "code/abstractmodule.h"

//class Theme;
class ShellModule : public AbstractModule
{
    Q_OBJECT
//    Q_PROPERTY(Theme* manager READ manager CONSTANT FINAL)
public:
    ShellModule(QObject * parent = nullptr);


    // AbstractModule interface
public:
    QString qmlSource() const override final;
    QString iconName() const override final;
    QString name() const override final;
    QString category() const override final;
    QStringList keywords() const override final;

    QString description() const override final;

//    Theme* manager();

private:
//    Theme *m_manager;


};

