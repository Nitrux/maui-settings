#ifndef MODULESMODEL_H
#define MODULESMODEL_H

#include <QAbstractListModel>

class ModulesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ModulesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override final;
    QVariant data(const QModelIndex &index, int role) const override final;
    QHash<int, QByteArray> roleNames() const override final;

private:
};

#endif // MODULESMODEL_H
