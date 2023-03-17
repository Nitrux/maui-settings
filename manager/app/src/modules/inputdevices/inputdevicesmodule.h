#pragma once

#include <QObject>
#include "code/abstractmodule.h"

namespace MauiMan
{
class InputDevicesManager;
}

class InputDevicesModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(MauiMan::InputDevicesManager* manager READ manager CONSTANT FINAL)

public:
    InputDevicesModule(QObject *parent = nullptr);
    MauiMan::InputDevicesManager* manager();

private:
    MauiMan::InputDevicesManager *m_manager;

Q_SIGNALS:

};

