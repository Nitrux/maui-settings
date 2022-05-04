#ifndef CONECTIONSMODEL_H
#define CONECTIONSMODEL_H

#include <QAbstractListModel>
#include <QTimer>

#include <KAuth>

class ConnectionsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ConnectionsModelRoles
    {
        ProtocolRole = Qt::UserRole + 1,
        LocalAddressRole,
        ForeignAddressRole,
        StatusRole,
        PidRole,
        ProgramRole
    };

    explicit ConnectionsModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

protected slots:
    void refreshConnections();

private:
    bool m_queryRunning;
    QVariantList m_connectionsData;
    QTimer timer;
};

#endif // CONECTIONSMODEL_H
