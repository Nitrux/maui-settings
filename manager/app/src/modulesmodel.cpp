#include "modulesmodel.h"
#include <QRegularExpression>

ModulesModel::ModulesModel(ModulesProxyModel *parent) : QAbstractListModel(parent)
  ,m_proxy(parent)
{

}

ModulesModel::~ModulesModel()
{
    qDeleteAll(m_modules);
}


int ModulesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_modules.count();
}

QVariant ModulesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch(role)
    {
    case 0: return QVariant::fromValue(m_modules.at(index.row()));
    default: return QVariant();
    }

    return QVariant();
}

QHash<int, QByteArray> ModulesModel::roleNames() const
{
    return {{0, "Module"}};
}

void ModulesModel::appendModule(AbstractModule *module)
{
    beginInsertRows(QModelIndex(), m_modules.count(), m_modules.count());
    m_modules << module;
    endInsertRows();
}

QVector<AbstractModule *> ModulesModel::modules() const
{
    return m_modules;
}

ModulesProxyModel::ModulesProxyModel(QObject *parent) :  QSortFilterProxyModel(parent)
  ,m_model(new ModulesModel(this))
{
    this->setSourceModel(m_model);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void ModulesProxyModel::setFilter(QString filter)
{
    if (this->m_filter == filter)
        return;

    this->m_filter = filter;
    this->setFilterFixedString(this->m_filter);
    Q_EMIT this->filterChanged(this->m_filter);
}

QString ModulesProxyModel::filter() const
{
    return m_filter;
}

void ModulesProxyModel::resetFilter()
{
    this->setFilterRegularExpression(QStringLiteral(""));
    this->invalidateFilter();
}

ModulesModel *ModulesProxyModel::model() const
{
    return m_model;
}

int ModulesProxyModel::count() const
{
    return m_model->rowCount(QModelIndex());
}

bool ModulesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const auto module = qvariant_cast<AbstractModule*>( this->sourceModel()->data(index, 0));
    return module->checkFilter(this->filterRegularExpression());

}

bool ModulesProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    return true;
}
