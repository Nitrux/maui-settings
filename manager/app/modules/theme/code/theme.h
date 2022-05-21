#pragma once

#include <QObject>
#include <QString>
#include <QQmlEngine>
#include <QDBusInterface>

class Theme : public QObject
{
    Q_OBJECT

public:
    explicit Theme(QObject * parent = nullptr);

};

