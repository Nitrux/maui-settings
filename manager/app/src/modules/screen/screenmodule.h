#pragma once

#include "code/abstractmodule.h"

class Screen;
class ScreenModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(Screen* manager READ manager CONSTANT FINAL)
public:
    ScreenModule(QObject * parent = nullptr);


    // AbstractModule interface
public:
    QString qmlSource() const override final;
    QString iconName() const override final;
    QString name() const override final;
    QString category() const override final;
    QStringList keywords() const override final;

    QString description() const override final;

    Screen* manager();

private:
    Screen *m_manager;


};

