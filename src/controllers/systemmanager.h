#pragma once

#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class QNetworkAccessManager;

class SystemManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString timezone READ timezone NOTIFY timezoneChanged)
    Q_PROPERTY(QStringList timezones READ timezones CONSTANT)
    Q_PROPERTY(QString locale READ locale NOTIFY localeChanged)
    Q_PROPERTY(QString hostName READ hostName NOTIFY hostNameChanged)
    Q_PROPERTY(QVariantList users READ users NOTIFY usersChanged)
    Q_PROPERTY(QStringList locales READ locales CONSTANT)
    Q_PROPERTY(bool automaticLocation READ automaticLocation WRITE setAutomaticLocation NOTIFY automaticLocationChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit SystemManager(QObject *parent = nullptr);

    QString timezone() const;
    QStringList timezones() const;
    QString locale() const;
    QString hostName() const;
    QVariantList users() const;
    QStringList locales() const;
    bool automaticLocation() const;
    bool busy() const;
    QString errorMessage() const;

    void setAutomaticLocation(bool enabled);

    Q_INVOKABLE void setTimezone(const QString &timezone);
    Q_INVOKABLE void setLocale(const QString &locale);
    Q_INVOKABLE void setHostName(const QString &hostName);
    Q_INVOKABLE void reloadUsers();
    Q_INVOKABLE void addUser(const QString &username, const QString &fullName, const QString &password, bool administrator, bool usePasswordQuality, const QString &avatarPath);
    Q_INVOKABLE void updateUser(const QString &username, const QString &password, const QString &avatarPath);
    Q_INVOKABLE void deleteUser(const QString &username);
    Q_INVOKABLE void setDateTime(const QString &isoDateTime);
    Q_INVOKABLE void lookupTimezoneByLocation();

Q_SIGNALS:
    void timezoneChanged();
    void localeChanged();
    void hostNameChanged();
    void usersChanged();
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
    QString m_locale;
    QString m_hostName;
    QVariantList m_users;
    QStringList m_locales;
    bool m_automaticLocation = false;
    bool m_busy = false;
    QString m_errorMessage;
    QNetworkAccessManager *m_network = nullptr;
};
