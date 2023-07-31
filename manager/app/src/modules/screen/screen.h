#pragma once
#include <QObject>

#include <MauiMan4/screenmanager.h>

class Screen : public MauiMan::ScreenManager
{
    Q_OBJECT    
public:
    explicit Screen(QObject *parent = nullptr);

};

