#pragma once

#include "code/abstractmodule.h"

class Background;
class BackgroundModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(Background* manager READ manager CONSTANT FINAL)
public:
    BackgroundModule(QObject * parent = nullptr);
    Background* manager();

private:
    Background *m_manager;
};

