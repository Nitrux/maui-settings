#pragma once

#include <QObject>
#include "modulesmodel.h"

class ModulesManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ModulesModel * model READ model CONSTANT FINAL)
    Q_PROPERTY(bool serverRunning READ serverRunning NOTIFY serverRunningChanged)
    Q_PROPERTY(bool isMauiSession READ isMauiSession CONSTANT FINAL)
    Q_PROPERTY(QString currentDesktopSession READ currentDesktopSession CONSTANT FINAL)

public:
    explicit ModulesManager(QObject *parent = nullptr);

    ModulesModel * model() const;

    bool serverRunning() const;

    bool isMauiSession() const;

    QString currentDesktopSession() const;

public slots:
    void startServer();
    AbstractModule *sourceFor(const QString &moduleId);

private:
    ModulesModel * m_model;

    bool m_serverRunning = false;

signals:

    void serverRunningChanged(bool serverRunning);
};

