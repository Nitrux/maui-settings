#pragma once

#include "code/abstractmodule.h"

class ScreenshotHandler;
class ScreenshotModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(ScreenshotHandler* manager READ manager CONSTANT FINAL)
public:
    ScreenshotModule(QObject * parent = nullptr);
    ScreenshotHandler* manager();

private:
    ScreenshotHandler *m_manager;


};

