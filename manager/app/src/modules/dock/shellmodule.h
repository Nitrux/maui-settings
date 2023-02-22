#pragma once

#include "code/abstractmodule.h"

//class Theme;
class ShellModule : public AbstractModule
{
    Q_OBJECT
//    Q_PROPERTY(Theme* manager READ manager CONSTANT FINAL)
public:
    ShellModule(QObject * parent = nullptr);



//    Theme* manager();

private:
//    Theme *m_manager;


};

