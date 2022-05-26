#pragma once

#include <QObject>
#include <QString>
#include <QQmlEngine>
#include <QDBusInterface>

#include "code/modules/thememanager.h"

class Theme : public MauiMan::ThemeManager
{
    Q_OBJECT
public:
    explicit Theme(QObject * parent = nullptr);

};

