#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

class FlatpakPermissionsController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(QVariantList applications READ applications NOTIFY applicationsChanged)
    Q_PROPERTY(QString selectedAppId READ selectedAppId NOTIFY selectedAppChanged)
    Q_PROPERTY(QString selectedAppName READ selectedAppName NOTIFY selectedAppChanged)
    Q_PROPERTY(QVariantList permissions READ permissions NOTIFY permissionsChanged)
    Q_PROPERTY(bool isSaveNeeded READ isSaveNeeded NOTIFY permissionsChanged)
    Q_PROPERTY(bool isDefaults READ isDefaults NOTIFY permissionsChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit FlatpakPermissionsController(QObject *parent = nullptr);

    bool available() const;
    QVariantList applications() const;
    QString selectedAppId() const;
    QString selectedAppName() const;
    QVariantList permissions() const;
    bool isSaveNeeded() const;
    bool isDefaults() const;
    QString errorMessage() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void selectApplication(const QString &appId);
    Q_INVOKABLE bool setPermission(int index, bool enabled);
    Q_INVOKABLE bool setPermissionValue(int index, const QString &value);
    Q_INVOKABLE bool addPermission(const QString &section, const QString &name, const QString &value);
    Q_INVOKABLE bool removePermission(int index);
    Q_INVOKABLE void defaults();
    Q_INVOKABLE bool save();
    Q_INVOKABLE void clearError();

    static QString normalizeContextEntry(const QString &entry, const QString &category);
    static QString filesystemMode(const QString &entry, QString *name = nullptr);

Q_SIGNALS:
    void availableChanged();
    void applicationsChanged();
    void selectedAppChanged();
    void permissionsChanged();
    void errorMessageChanged();

private:
    struct Application
    {
        QString appId;
        QString displayName;
        QString baseDirectory;
        QString metadataPath;
    };

    static QStringList flatpakBaseDirectories();
    static QString permissionKey(const QVariantMap &permission);
    static QString formatFilesystemEntry(const QString &name, const QString &mode);
    static QVariantList buildPermissions(const class KConfig &defaults, const class KConfig &effective);

    void loadApplications();
    void loadSelectedApplication();
    void setErrorMessage(const QString &message);
    bool validPermissionName(const QString &section, const QString &name) const;
    bool hasPermission(const QString &section, const QString &name) const;
    QVariantMap defaultPermission(const QVariantMap &permission) const;

    bool m_available = false;
    QList<Application> m_applicationData;
    QVariantList m_applications;
    QString m_selectedAppId;
    QString m_selectedAppName;
    QVariantList m_permissions;
    QVariantList m_defaultPermissions;
    QVariantList m_savedPermissions;
    QString m_userOverridePath;
    QString m_errorMessage;
};
