#pragma once
#include <QObject>

class DBusModuleSyncer : public QObject
{
    Q_OBJECT
public:
    DBusModuleSyncer(QObject * parent = nullptr);
};

