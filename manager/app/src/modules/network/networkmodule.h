#pragma once

#include "code/abstractmodule.h"

//class Theme;
class NetworkModule : public AbstractModule
{
    Q_OBJECT
//    Q_PROPERTY(Theme* manager READ manager CONSTANT FINAL)
public:
    NetworkModule(QObject * parent = nullptr);


//    Theme* manager();

private:
//    Theme *m_manager;


};

