#pragma once

#include <QObject>
#include <QStringList>
#include <QVariantMap>

class QNetworkAccessManager;

class DateTimeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString timezone READ timezone NOTIFY timezoneChanged)
    Q_PROPERTY(QStringList timezones READ timezones CONSTANT)
    Q_PROPERTY(bool automaticLocation READ automaticLocation WRITE setAutomaticLocation NOTIFY automaticLocationChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit DateTimeManager(QObject *parent = nullptr);

    QString timezone() const;
    QStringList timezones() const;
    bool automaticLocation() const;
    bool busy() const;
    QString errorMessage() const;

    void setAutomaticLocation(bool enabled);

    Q_INVOKABLE void setTimezone(const QString &timezone);
    Q_INVOKABLE void setDateTime(const QString &isoDateTime);
    Q_INVOKABLE void lookupTimezoneByLocation();

Q_SIGNALS:
    void timezoneChanged();
    void automaticLocationChanged();
    void busyChanged();
    void errorMessageChanged();
    void operationSucceeded(const QString &message);

private:
    void execute(const QVariantMap &arguments, const QString &successMessage);
    void setBusy(bool busy);
    void setErrorMessage(const QString &message);

    QString m_timezone;
    QStringList m_timezones;
    bool m_automaticLocation = false;
    bool m_busy = false;
    QString m_errorMessage;
    QNetworkAccessManager *m_network = nullptr;
};
