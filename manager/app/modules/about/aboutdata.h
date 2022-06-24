#ifndef ABOUTDATA_H
#define ABOUTDATA_H

#include <QObject>

class AboutData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString productVersion READ productVersion CONSTANT FINAL)
    Q_PROPERTY(QString productName READ productName CONSTANT FINAL)
    Q_PROPERTY(QString productType READ productType CONSTANT FINAL)
    Q_PROPERTY(QString machineName READ machineName CONSTANT FINAL)
    Q_PROPERTY(QString kernelType READ kernelType CONSTANT FINAL)
    Q_PROPERTY(QString kernelVersion READ kernelVersion CONSTANT FINAL)
    Q_PROPERTY(QString cpuArch READ cpuArch CONSTANT FINAL)
    Q_PROPERTY(QString cpuInfo READ cpuInfo CONSTANT FINAL)
    Q_PROPERTY(QString userName READ userName CONSTANT FINAL)
    Q_PROPERTY(QString distroName READ distroName  CONSTANT FINAL)
    Q_PROPERTY(QString distroWebPage READ distroWebPage  CONSTANT FINAL)
    Q_PROPERTY(QString distroVersion READ distroVersion  CONSTANT FINAL)

    QString m_cpuInfo;

    QString m_userName;

    QString m_distroWebPage;

    QString m_distroName;

    QString m_distroVersion;

public:
    explicit AboutData(QObject *parent = nullptr);

    QString productVersion() const;

    QString productName() const;

    QString productType() const;

    QString machineName() const;

    QString kernelType() const;

    QString kernelVersion() const;

    QString cpuArch() const;

    QString cpuInfo() const;

    QString userName() const;

    QString distroWebPage() const;

    QString distroName() const;

    QString distroVersion() const;

signals:

};

#endif // ABOUTDATA_H
