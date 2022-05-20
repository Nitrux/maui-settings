#pragma once

#include "code/abstractmodule.h"

class BackgroundManager;
class BackgroundModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(BackgroundManager* manager READ manager CONSTANT FINAL)
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


    BackgroundManager* manager() const;

private:
    BackgroundManager *m_manager;


};

