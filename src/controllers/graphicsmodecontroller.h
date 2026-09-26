#pragma once

#include <QObject>
#include <QString>

class GraphicsModeController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY stateChanged)
    Q_PROPERTY(QString currentMode READ currentMode NOTIFY stateChanged)
    Q_PROPERTY(QString pendingMode READ pendingMode NOTIFY stateChanged)
    Q_PROPERTY(bool rebootRequired READ rebootRequired NOTIFY stateChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY stateChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY stateChanged)

public:
    explicit GraphicsModeController(QObject *parent = nullptr);

    bool available() const;
    QString currentMode() const;
    QString pendingMode() const;
    bool rebootRequired() const;
    bool busy() const;
    QString errorMessage() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void setMode(const QString &mode);

Q_SIGNALS:
    void stateChanged();

private:
    bool detectSupportedHardware() const;
    QString queryCurrentMode() const;
    void setBusy(bool value);
    void setErrorMessage(const QString &message);

    bool m_available = false;
    QString m_currentMode;
    QString m_pendingMode;
    bool m_rebootRequired = false;
    bool m_busy = false;
    QString m_errorMessage;
};
