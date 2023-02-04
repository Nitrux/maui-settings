#pragma once

#include "code/abstractmodule.h"

class Theme;
class ThemeModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(Theme* manager READ manager CONSTANT FINAL)
public:
    ThemeModule(QObject * parent = nullptr);
    Theme* manager();

private:
    Theme *m_manager;

};

