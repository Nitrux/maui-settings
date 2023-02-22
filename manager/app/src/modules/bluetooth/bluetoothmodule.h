#pragma once

#include "code/abstractmodule.h"

//class Theme;
class BluetoothModule : public AbstractModule
{
    Q_OBJECT
//    Q_PROPERTY(Theme* manager READ manager CONSTANT FINAL)

public:
    BluetoothModule(QObject * parent = nullptr);

//    Theme* manager();

private:
//    Theme *m_manager;


};

