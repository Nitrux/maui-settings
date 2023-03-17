#pragma once

#include <QObject>
#include "modulesmodel.h"

class ModulesManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ModulesProxyModel * model READ model CONSTANT FINAL)
    Q_PROPERTY(bool serverRunning READ serverRunning NOTIFY serverRunningChanged)
    Q_PROPERTY(bool isMauiSession READ isMauiSession CONSTANT FINAL)
    Q_PROPERTY(bool caskServerRunning READ caskServerRunning NOTIFY caskServerRunningChanged)
    Q_PROPERTY(QString currentDesktopSession READ currentDesktopSession CONSTANT FINAL)

public:
    explicit ModulesManager(QObject *parent = nullptr);

    ModulesProxyModel * model() const;

    bool serverRunning() const;

    bool isMauiSession() const;

    QString currentDesktopSession() const;

    bool caskServerRunning() const;

public slots:
    void startMauiManServer();
    void startCaskServer();

    AbstractModule *sourceFor(const QString &moduleId);

private:
    ModulesProxyModel * m_model;

    bool m_serverRunning = false;

    bool m_caskServerRunning = false;

signals:

    void serverRunningChanged(bool serverRunning);
    void caskServerRunningChanged(bool caskServerRunning);
};

