#pragma once
#include <QObject>

#include <MauiMan/screenmanager.h>

class Screen : public MauiMan::ScreenManager
{
    Q_OBJECT    
public:
    explicit Screen(QObject *parent = nullptr);

signals:

};

