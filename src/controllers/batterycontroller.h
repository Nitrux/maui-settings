#pragma once

#include <QList>
#include <QObject>
#include <QVariantList>

class BatteryController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY dataChanged)
    Q_PROPERTY(bool hasBattery READ hasBattery NOTIFY dataChanged)
    Q_PROPERTY(int percentage READ percentage NOTIFY dataChanged)
    Q_PROPERTY(QString state READ state NOTIFY dataChanged)
    Q_PROPERTY(bool charging READ charging NOTIFY dataChanged)
    Q_PROPERTY(bool pluggedIn READ pluggedIn NOTIFY dataChanged)
    Q_PROPERTY(bool onBattery READ onBattery NOTIFY dataChanged)
    Q_PROPERTY(qint64 timeToEmpty READ timeToEmpty NOTIFY dataChanged)
    Q_PROPERTY(qint64 timeToFull READ timeToFull NOTIFY dataChanged)
    Q_PROPERTY(int health READ health NOTIFY dataChanged)
    Q_PROPERTY(QString capacityLevel READ capacityLevel NOTIFY dataChanged)
    Q_PROPERTY(QString technology READ technology NOTIFY dataChanged)
    Q_PROPERTY(double energy READ energy NOTIFY dataChanged)
    Q_PROPERTY(double energyFull READ energyFull NOTIFY dataChanged)
    Q_PROPERTY(double energyFullDesign READ energyFullDesign NOTIFY dataChanged)
    Q_PROPERTY(double energyRate READ energyRate NOTIFY dataChanged)
    Q_PROPERTY(double voltage READ voltage NOTIFY dataChanged)
    Q_PROPERTY(int chargeCycles READ chargeCycles NOTIFY dataChanged)
    Q_PROPERTY(QString vendor READ vendor NOTIFY dataChanged)
    Q_PROPERTY(QString model READ model NOTIFY dataChanged)
    Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)

public:
    explicit BatteryController(QObject *parent = nullptr);

    bool available() const;
    bool hasBattery() const;
    int percentage() const;
    QString state() const;
    bool charging() const;
    bool pluggedIn() const;
    bool onBattery() const;
    qint64 timeToEmpty() const;
    qint64 timeToFull() const;
    int health() const;
    QString capacityLevel() const;
    QString technology() const;
    double energy() const;
    double energyFull() const;
    double energyFullDesign() const;
    double energyRate() const;
    double voltage() const;
    int chargeCycles() const;
    QString vendor() const;
    QString model() const;
    QVariantList history() const;

    Q_INVOKABLE void refresh();

Q_SIGNALS:
    void dataChanged();
    void historyChanged();

private:
    struct HistoryEntry {
        qint64 timestamp = 0;
        int percentage = 0;
        bool charging = false;
        double dischargeRate = 0.0;
    };

    void loadHistory();
    void saveHistory() const;
    void pruneHistory(qint64 now);
    bool addHistorySample(qint64 now);
    QVariantList historyVariantList() const;

    bool m_available = false;
    bool m_hasBattery = false;
    int m_percentage = 0;
    QString m_state;
    bool m_charging = false;
    bool m_pluggedIn = false;
    bool m_onBattery = false;
    qint64 m_timeToEmpty = 0;
    qint64 m_timeToFull = 0;
    int m_health = 0;
    QString m_capacityLevel;
    QString m_technology;
    double m_energy = 0.0;
    double m_energyFull = 0.0;
    double m_energyFullDesign = 0.0;
    double m_energyRate = 0.0;
    double m_voltage = 0.0;
    int m_chargeCycles = -1;
    QString m_vendor;
    QString m_model;
    QList<HistoryEntry> m_history;
    QString m_historyPath;
};
