#pragma once

#include <QObject>
#include <QUrl>

#include "mauiman_export.h"

class MAUIMAN_EXPORT BackgroundMan : public QObject
{
    Q_OBJECT

public:
    BackgroundMan(QObject * parent = nullptr);

};
