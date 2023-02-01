#pragma once

#include "code/abstractmodule.h"

class AboutData;
class AboutModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(AboutData* manager READ manager CONSTANT FINAL)

public:
    AboutModule(QObject * parent = nullptr);

    // AbstractModule interface
public:
    QString qmlSource() const override final;
    QString iconName() const override final;
    QString name() const override final;
    QString category() const override final;
    QStringList keywords() const override final;

    QString description() const override final;

    AboutData* manager();

private:
    AboutData *m_manager;


};

