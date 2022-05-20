#include "modulesmodel.h"

ModulesModel::ModulesModel(QObject *parent) : QAbstractListModel(parent)
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
