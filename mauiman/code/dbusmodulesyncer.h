#pragma once
#include <QObject>
#include "mauiman_export.h"

class MAUIMAN_EXPORT DBusModuleSyncer : public QObject
{
    Q_OBJECT
public:
    DBusModuleSyncer(QObject * parent = nullptr);
};

