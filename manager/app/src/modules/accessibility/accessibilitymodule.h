#pragma once
#include <QObject>
#include "code/abstractmodule.h"

namespace MauiMan
{
class AccessibilityManager;
}


class AccessibilityModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(MauiMan::AccessibilityManager* manager READ manager CONSTANT FINAL)

public:
    explicit AccessibilityModule(QObject *parent = nullptr);

signals:


    // AbstractModule interface
public:
    QString qmlSource() const override final;
    QString iconName() const override final;
    QString name() const override final;
    QString category() const override final;
    QStringList keywords() const override final;
    QString description() const override final;

    MauiMan::AccessibilityManager* manager();

private:
    MauiMan::AccessibilityManager *m_manager;
};

