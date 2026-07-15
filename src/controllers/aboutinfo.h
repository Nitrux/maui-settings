#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class AboutInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString osDistribution READ osDistribution CONSTANT)
    Q_PROPERTY(QString osVersion READ osVersion CONSTANT)
    Q_PROPERTY(QString osKernel READ osKernel CONSTANT)
    Q_PROPERTY(QString osSession READ osSession CONSTANT)
    Q_PROPERTY(QString hostName READ hostName CONSTANT)
    Q_PROPERTY(QString cpuModel READ cpuModel CONSTANT)
    Q_PROPERTY(QString cpuArchitecture READ cpuArchitecture CONSTANT)
    Q_PROPERTY(QString memoryTotal READ memoryTotal CONSTANT)
    Q_PROPERTY(QString memoryAvailable READ memoryAvailable CONSTANT)
    Q_PROPERTY(QVariantList storageVolumes READ storageVolumes CONSTANT)
    Q_PROPERTY(QString storageMountPoint READ storageMountPoint CONSTANT)
    Q_PROPERTY(QString storageFileSystem READ storageFileSystem CONSTANT)
    Q_PROPERTY(QString storageUsed READ storageUsed CONSTANT)
    Q_PROPERTY(QString storageAvailable READ storageAvailable CONSTANT)
    Q_PROPERTY(QString storageTotal READ storageTotal CONSTANT)

public:
    explicit AboutInfo(QObject *parent = nullptr);

    QString osDistribution() const;
    QString osVersion() const;
    QString osKernel() const;
    QString osSession() const;
    QString hostName() const;
    QString cpuModel() const;
    QString cpuArchitecture() const;
    QString memoryTotal() const;
    QString memoryAvailable() const;
    QVariantList storageVolumes() const;
    QString storageMountPoint() const;
    QString storageFileSystem() const;
    QString storageUsed() const;
    QString storageAvailable() const;
    QString storageTotal() const;

private:
    static QString formatBytes(qint64 bytes);
    static QString readReleaseValue(const QString &path, const QString &key);
    static qint64 readMemInfoValueKB(const QString &key);
    static QString readCpuModel();
};
