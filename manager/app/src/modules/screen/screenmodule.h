#pragma once

#include "code/abstractmodule.h"

class Screen;
class ScreenModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(Screen* manager READ manager CONSTANT FINAL)
public:
    ScreenModule(QObject * parent = nullptr);
    Screen* manager();

private:
    Screen *m_manager;


};

