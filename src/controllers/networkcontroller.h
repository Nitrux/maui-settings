#pragma once

#include <QAbstractListModel>
#include <QVariantList>
#include <QVariantMap>

class NetworkController : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool wirelessEnabled READ wirelessEnabled WRITE setWirelessEnabled NOTIFY wirelessEnabledChanged)
    Q_PROPERTY(bool hardwareEnabled READ hardwareEnabled NOTIFY hardwareEnabledChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(QVariantList wiredConnections READ wiredConnections NOTIFY wiredConnectionsChanged)
    Q_PROPERTY(QVariantMap connectedWirelessConnection READ connectedWirelessConnection NOTIFY connectedWirelessConnectionChanged)

public:
    enum Role {
        SsidRole = Qt::UserRole + 1,
        SignalStrengthRole,
        SecurityRole,
        SecureRole,
        PasswordRequiredRole,
        ConnectedRole,
        SavedRole,
        AutoConnectRole,
        DevicePathRole,
        AccessPointPathRole,
        ConnectionUuidRole,
    };
    Q_ENUM(Role)

    explicit NetworkController(QObject *parent = nullptr);
    ~NetworkController() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool available() const;
    bool wirelessEnabled() const;
    void setWirelessEnabled(bool enabled);
    bool hardwareEnabled() const;
    bool scanning() const;
    QString errorMessage() const;
    QVariantList wiredConnections() const;
    QVariantMap connectedWirelessConnection() const;

    Q_INVOKABLE void requestScan();
    Q_INVOKABLE void connectToNetwork(const QString &devicePath,
                                      const QString &accessPointPath,
                                      const QString &ssid,
                                      const QString &password);
    Q_INVOKABLE void disconnectNetwork(const QString &devicePath);
    Q_INVOKABLE void connectWired(const QString &devicePath, const QString &connectionUuid);
    Q_INVOKABLE void updateSavedNetwork(const QString &devicePath,
                                        const QString &ssid,
                                        bool autoConnect);
    Q_INVOKABLE void forgetNetwork(const QString &devicePath, const QString &ssid);
    Q_INVOKABLE void updateConnection(const QString &connectionUuid, const QVariantMap &values);
    Q_INVOKABLE void clearError();

Q_SIGNALS:
    void availableChanged();
    void wirelessEnabledChanged();
    void hardwareEnabledChanged();
    void scanningChanged();
    void errorMessageChanged();
    void wiredConnectionsChanged();
    void connectedWirelessConnectionChanged();

private:
    class Private;
    Private *const d;

    void rebuild();
    void scheduleRebuild();
    void setScanning(bool scanning);
    void setErrorMessage(const QString &message);
};
