#pragma once

#include "code/abstractmodule.h"
#include <QFont>

class Theme;
class ThemeModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(Theme* manager READ manager CONSTANT FINAL)
public:
    ThemeModule(QObject * parent = nullptr);
    Theme* manager();

public slots:
    QFont getFont(const QString &desc);
QString fontToString(const QFont &font);
QStringList fontStyles(const QFont &font);
QStringList fontPointSizes(const QFont &font);

private:
    Theme *m_manager;
    void updateGtk3Config();
};

