#pragma once
#include <QObject>
#include <QString>
class PowerInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY settingsChanged)
    Q_PROPERTY(int powerSaverMax READ powerSaverMax WRITE setPowerSaverMax NOTIFY settingsChanged)
    Q_PROPERTY(int balancedMax READ balancedMax WRITE setBalancedMax NOTIFY settingsChanged)
    Q_PROPERTY(int performanceMin READ performanceMin WRITE setPerformanceMin NOTIFY settingsChanged)
    Q_PROPERTY(bool daemonAvailable READ daemonAvailable NOTIFY settingsChanged)
    Q_PROPERTY(bool daemonRunning READ daemonRunning NOTIFY settingsChanged)
    Q_PROPERTY(bool configAvailable READ configAvailable NOTIFY settingsChanged)
public:
    explicit PowerInfo(QObject *parent = nullptr);
    QString configPath() const;
    bool enabled() const;
    int powerSaverMax() const;
    int balancedMax() const;
    int performanceMin() const;
    bool daemonAvailable() const;
    bool daemonRunning() const;
    bool configAvailable() const;
    void setEnabled(bool value);
    void setPowerSaverMax(int value);
    void setBalancedMax(int value);
    void setPerformanceMin(int value);
    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();
Q_SIGNALS:
    void settingsChanged();
private:
    void load();
    QString m_configPath;
    bool m_enabled = true;
    int m_powerSaverMax = 20;
    int m_balancedMax = 59;
    int m_performanceMin = 60;
    bool m_daemonAvailable = false;
    bool m_daemonRunning = false;
    bool m_configAvailable = false;
    bool daemonIsRunning() const;
};