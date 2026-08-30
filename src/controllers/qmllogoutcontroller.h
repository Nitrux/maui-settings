#pragma once

#include <QObject>
#include <QString>

class QmlLogoutController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(QString iconMode READ iconMode WRITE setIconMode NOTIFY settingsChanged)
    Q_PROPERTY(double overlayOpacity READ overlayOpacity WRITE setOverlayOpacity NOTIFY settingsChanged)
    Q_PROPERTY(QString avatarPath READ avatarPath WRITE setAvatarPath NOTIFY settingsChanged)
    Q_PROPERTY(bool showUptime READ showUptime WRITE setShowUptime NOTIFY settingsChanged)
    Q_PROPERTY(int actionTimeout READ actionTimeout WRITE setActionTimeout NOTIFY settingsChanged)

public:
    explicit QmlLogoutController(QObject *parent = nullptr);

    QString configPath() const;
    bool available() const;
    QString iconMode() const;
    double overlayOpacity() const;
    QString avatarPath() const;
    bool showUptime() const;
    int actionTimeout() const;

    void setIconMode(const QString &value);
    void setOverlayOpacity(double value);
    void setAvatarPath(const QString &value);
    void setShowUptime(bool value);
    void setActionTimeout(int value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    void setChanged();

    QString m_configPath;
    bool m_available = false;
    QString m_iconMode = QStringLiteral("system");
    double m_overlayOpacity = 0.76;
    QString m_avatarPath;
    bool m_showUptime = true;
    int m_actionTimeout = 30;
};
