#pragma once
#include <QObject>

#if (defined Q_OS_LINUX || defined Q_OS_FREEBSD) && !defined Q_OS_ANDROID
class OrgMauiSettingsActionsInterface;

namespace AppInstance
{
QVector<QPair<QSharedPointer<OrgMauiSettingsActionsInterface>, QStringList> > appInstances(const QString& preferredService);

bool attachToExistingInstance(const QString &module, const QString& preferredService = QString());

bool registerService();
}
#endif

class Server : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.maui.settings.Actions")

public:
    explicit Server(QObject *parent = nullptr);
    void setQmlObject(QObject  *object);

public Q_SLOTS:
    /**
           * Tries to raise/activate the Dolphin window.
           */
    void activateWindow();

    /** Stores all settings and quits Dolphin. */
    void quit();

    void openModule(const QString &module);
    /**
                * Determines if a URL is open in any tab.
                * @note Use of QString instead of QUrl is required to be callable via DBus.
                *
                * @param url URL to look for
                * @returns true if url is currently open in a tab, false otherwise.
                */
    bool isModuleOpen(const QString &module);


private:
    QObject* m_qmlObject = nullptr;
    QStringList filterFiles(const QStringList &urls);
};

