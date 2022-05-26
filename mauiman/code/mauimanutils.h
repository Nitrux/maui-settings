#ifndef MAUIMANUTILS_H
#define MAUIMANUTILS_H

#include <QObject>
#include "mauiman_export.h"

class MAUIMAN_EXPORT MauiManUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool serverRunning READ serverRunning NOTIFY serverRunningChanged)

public:
    explicit MauiManUtils(QObject *parent = nullptr);

    bool serverRunning() const;

    static void startServer();
    static void invokeManager(const QString &module);

signals:
    void serverRunningChanged(bool state);

private:
    bool m_serverRunning = false;
};

#endif // MAUIMANUTILS_H
