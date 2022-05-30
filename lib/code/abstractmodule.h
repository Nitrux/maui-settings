#pragma once

#include <QObject>
#include <QUrl>

#include "mauisettingslib_export.h"

class MAUISETTINGSLIB_EXPORT AbstractModule : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName NOTIFY iconNameChanged)
    Q_PROPERTY(QString  name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString category READ category NOTIFY categoryChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QStringList keywords READ keywords NOTIFY keywordsChanged)
    Q_PROPERTY(QString qmlSource READ qmlSource NOTIFY qmlSourceChanged)

public:
    AbstractModule(QObject * parent = nullptr);

    virtual QString qmlSource() const = 0;
    virtual QString iconName() const = 0;
    virtual QString name() const = 0;
    virtual QString category() const = 0;
    virtual QStringList keywords() const = 0;
    virtual QString description() const = 0;

private:
//    QUrl m_qmlSource;
//    QString m_name;
//    QString m_category;
//    QStringList m_keywords;
//    QString m_iconName;

signals:
    void iconNameChanged();
    void nameChanged();
    void descriptionChanged();
    void categoryChanged();
    void keywordsChanged();
    void qmlSourceChanged();
};

