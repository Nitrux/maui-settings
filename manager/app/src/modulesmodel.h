#pragma once

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "code/abstractmodule.h"

class ModulesModel;
class ModulesProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged RESET resetFilter)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit ModulesProxyModel(QObject *parent = nullptr );
    void setFilter(QString filter);

    QString filter() const;
    void resetFilter();

    ModulesModel * model() const;
    int count() const;

Q_SIGNALS:
    void filterChanged(QString filter);

    void countChanged(int count);

private:
    QString m_filter;
    mutable ModulesModel *m_model;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override final;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override final;
};


class ModulesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ModulesModel(ModulesProxyModel *parent = nullptr);
    ~ModulesModel();

    int rowCount(const QModelIndex &parent) const override final;
    QVariant data(const QModelIndex &index, int role) const override final;
    QHash<int, QByteArray> roleNames() const override final;

    void appendModule(AbstractModule * module);

    QVector<AbstractModule *> modules() const;

private:
    QVector<AbstractModule*> m_modules;
    ModulesProxyModel *m_proxy;
};

