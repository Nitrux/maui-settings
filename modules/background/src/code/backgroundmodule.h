#pragma once

#include "mauiman/code/abstractmodule.h"

class BackgroundModule : public AbstractModule
{
    Q_OBJECT
public:
    BackgroundModule(QObject * parent = nullptr);

    // AbstractModule interface
public:
    QUrl qmlSource() const override final;
    QString iconName() const override final;
    QString name() const override final;
    QString category() const override final;
    QStringList keywords() const override final;
};

