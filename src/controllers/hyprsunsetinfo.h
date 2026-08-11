#pragma once

#include <QObject>
#include <QString>

class HyprsunsetInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(bool available READ available NOTIFY settingsChanged)
    Q_PROPERTY(bool configAvailable READ configAvailable NOTIFY settingsChanged)
    Q_PROPERTY(int maxGamma READ maxGamma WRITE setMaxGamma NOTIFY settingsChanged)
    Q_PROPERTY(int daytimeHour READ daytimeHour WRITE setDaytimeHour NOTIFY settingsChanged)
    Q_PROPERTY(int daytimeMinute READ daytimeMinute WRITE setDaytimeMinute NOTIFY settingsChanged)
    Q_PROPERTY(bool daytimeIdentity READ daytimeIdentity WRITE setDaytimeIdentity NOTIFY settingsChanged)
    Q_PROPERTY(int nighttimeHour READ nighttimeHour WRITE setNighttimeHour NOTIFY settingsChanged)
    Q_PROPERTY(int nighttimeMinute READ nighttimeMinute WRITE setNighttimeMinute NOTIFY settingsChanged)
    Q_PROPERTY(int nighttimeTemperature READ nighttimeTemperature WRITE setNighttimeTemperature NOTIFY settingsChanged)
    Q_PROPERTY(int nighttimeGammaPercent READ nighttimeGammaPercent WRITE setNighttimeGammaPercent NOTIFY settingsChanged)

public:
    explicit HyprsunsetInfo(QObject *parent = nullptr);

    QString configPath() const;
    bool available() const;
    bool configAvailable() const;
    int maxGamma() const;
    int daytimeHour() const;
    int daytimeMinute() const;
    bool daytimeIdentity() const;
    int nighttimeHour() const;
    int nighttimeMinute() const;
    int nighttimeTemperature() const;
    int nighttimeGammaPercent() const;

    void setMaxGamma(int value);
    void setDaytimeHour(int value);
    void setDaytimeMinute(int value);
    void setDaytimeIdentity(bool value);
    void setNighttimeHour(int value);
    void setNighttimeMinute(int value);
    void setNighttimeTemperature(int value);
    void setNighttimeGammaPercent(int value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    void setChanged();

    QString m_configPath;
    bool m_available = false;
    bool m_configAvailable = false;
    int m_maxGamma = 150;
    int m_daytimeHour = 7;
    int m_daytimeMinute = 0;
    bool m_daytimeIdentity = true;
    int m_nighttimeHour = 19;
    int m_nighttimeMinute = 0;
    int m_nighttimeTemperature = 5800;
    int m_nighttimeGammaPercent = 80;
};
