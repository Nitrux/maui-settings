#pragma once

#include "code/abstractmodule.h"
#include "aboutdata.h"

class AboutModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(AboutData* manager READ manager CONSTANT FINAL)

public:
    AboutModule(QObject * parent = nullptr);
    AboutData* manager();

private:
    AboutData *m_manager;
};

