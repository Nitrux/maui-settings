#pragma once
#include <QObject>
#include "code/abstractmodule.h"

#include <MauiMan4/accessibilitymanager.h>

class AccessibilityModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(MauiMan::AccessibilityManager* manager READ manager CONSTANT FINAL)

public:
    explicit AccessibilityModule(QObject *parent = nullptr);

    MauiMan::AccessibilityManager* manager();

private:
    MauiMan::AccessibilityManager *m_manager;
};

