#pragma once

#include "code/abstractmodule.h"

class BackgroundModule : public AbstractModule
{
    Q_OBJECT
public:
    BackgroundModule(QObject * parent = nullptr);

    // AbstractModule interface
public:
    QString qmlSource() const override final;
    QString iconName() const override final;
    QString name() const override final;
    QString category() const override final;
    QStringList keywords() const override final;

    QString description() const override final;
};

