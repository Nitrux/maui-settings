#pragma once

#include <QObject>
#include <QUrl>
#include <QRegularExpression>

#include "mauisettingslib_export.h"

class MAUISETTINGSLIB_EXPORT AbstractModule : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString iconName READ iconName CONSTANT FINAL)
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(QString category READ category CONSTANT FINAL)
    Q_PROPERTY(QString description READ description CONSTANT FINAL)
    Q_PROPERTY(QStringList keywords READ keywords CONSTANT FINAL)
    Q_PROPERTY(QString qmlSource READ qmlSource CONSTANT FINAL)
    Q_PROPERTY(QString id READ id CONSTANT FINAL)

public:
    AbstractModule(const QString &id, const QString &name, const QString &category, const QString &qmlSource, const QString &iconName = QStringLiteral("preferences"), const QString &description = QStringLiteral(""), const QStringList &keywords = {}, QObject * parent = nullptr);

    QString qmlSource() const
    {
        return m_qmlSource;
    }

    QString iconName() const
    {
        return m_iconName;
    }

    QString name() const
    {
        return m_name;
    }

    QString category() const
    {
        return m_category;
    }

    QStringList keywords() const
    {
        return m_keywords;
    }

    QString description()  const
    {
        return m_description;
    }

    QString id() const
    {
        return m_id;
    }

    bool checkFilter(const QRegularExpression &filter);

private:
    QString m_id;
    QString m_qmlSource;
    QString m_name;
    QString m_category;
    QStringList m_keywords;
    QString m_iconName;
    QString m_description;

Q_SIGNALS:
    void iconNameChanged();
    void nameChanged();
    void descriptionChanged();
    void categoryChanged();
    void keywordsChanged();
    void qmlSourceChanged();
};

