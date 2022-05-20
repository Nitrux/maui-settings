#ifndef MODULESMODEL_H
#define MODULESMODEL_H

#include <QAbstractListModel>
#include "code/abstractmodule.h"

class ModulesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ModulesModel(QObject *parent = nullptr);
~ModulesModel();
    int rowCount(const QModelIndex &parent) const override final;
    QVariant data(const QModelIndex &index, int role) const override final;
    QHash<int, QByteArray> roleNames() const override final;

    void appendModule(AbstractModule * module);
private:
    QVector<AbstractModule*> m_modules;
};

#endif // MODULESMODEL_H
