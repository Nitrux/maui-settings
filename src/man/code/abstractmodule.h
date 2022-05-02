#pragma once

#include <QObject>
#include <QUrl>

#include "mauiman_export.h"

class MAUIMAN_EXPORT AbstractModule : public QObject
{
    Q_OBJECT

public:
    AbstractModule(QObject * parent = nullptr);

    virtual QUrl qmlSource() const = 0;
    virtual QString iconName() const = 0;
    virtual QString name() const = 0;
    virtual QString category() const = 0;
    virtual QStringList keywords() const = 0;

private:
//    QUrl m_qmlSource;
//    QString m_name;
//    QString m_category;
//    QStringList m_keywords;
//    QString m_iconName;

};

