// Copyright 2026 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QAbstractListModel>

class BluetoothController : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool powered READ powered WRITE setPowered NOTIFY poweredChanged)
    Q_PROPERTY(bool discovering READ discovering NOTIFY discoveringChanged)
    Q_PROPERTY(QString adapterName READ adapterName NOTIFY adapterNameChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool pairingPromptActive READ pairingPromptActive NOTIFY pairingPromptChanged)
    Q_PROPERTY(bool pairingConfirmationRequired READ pairingConfirmationRequired NOTIFY pairingPromptChanged)
    Q_PROPERTY(QString pairingDeviceName READ pairingDeviceName NOTIFY pairingPromptChanged)
    Q_PROPERTY(QString pairingDeviceAddress READ pairingDeviceAddress NOTIFY pairingPromptChanged)
    Q_PROPERTY(QString pairingCode READ pairingCode NOTIFY pairingPromptChanged)
    Q_PROPERTY(QString pairingServiceUuid READ pairingServiceUuid NOTIFY pairingPromptChanged)
    Q_PROPERTY(QString pairingRequestId READ pairingRequestId NOTIFY pairingPromptChanged)

public:
    enum Role {
        NameRole = Qt::UserRole + 1,
        AddressRole,
        IconNameRole,
        PairedRole,
        ConnectedRole,
        TrustedRole,
        SignalStrengthRole,
        DevicePathRole,
    };
    Q_ENUM(Role)

    explicit BluetoothController(QObject *parent = nullptr);
    ~BluetoothController() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool available() const;
    bool powered() const;
    void setPowered(bool powered);
    bool discovering() const;
    QString adapterName() const;
    QString errorMessage() const;
    bool pairingPromptActive() const;
    bool pairingConfirmationRequired() const;
    QString pairingDeviceName() const;
    QString pairingDeviceAddress() const;
    QString pairingCode() const;
    QString pairingServiceUuid() const;
    QString pairingRequestId() const;

    Q_INVOKABLE void setDiscoveryEnabled(bool enabled);
    Q_INVOKABLE void pairDevice(const QString &devicePath);
    Q_INVOKABLE void unpairDevice(const QString &devicePath);
    Q_INVOKABLE void connectDevice(const QString &devicePath);
    Q_INVOKABLE void disconnectDevice(const QString &devicePath);
    Q_INVOKABLE void updateDevice(const QString &devicePath,
                                  const QString &name,
                                  bool trusted);
    Q_INVOKABLE void respondToPairingPrompt(const QString &requestId, bool accepted);
    Q_INVOKABLE void dismissPairingPrompt();
    Q_INVOKABLE void clearError();

Q_SIGNALS:
    void availableChanged();
    void poweredChanged();
    void discoveringChanged();
    void adapterNameChanged();
    void errorMessageChanged();
    void pairingPromptChanged();

private:
    class Private;
    Private *const d;

    void rebuild();
    void scheduleRebuild();
    void watchCall(QObject *call, bool ignoreCancellation = false);
    void setPairingPrompt(const QString &deviceName,
                          const QString &deviceAddress,
                          const QString &code,
                          const QString &serviceUuid,
                          const QString &requestId,
                          bool confirmationRequired);
    void setErrorMessage(const QString &message);
};
