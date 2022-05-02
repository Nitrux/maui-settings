#include "modulesmodel.h"

ModulesModel::ModulesModel(QObject *parent) : QAbstractListModel(parent)
{

}


int ModulesModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}

QVariant ModulesModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QHash<int, QByteArray> ModulesModel::roleNames() const
{
    return {{0, "module"}};
}
