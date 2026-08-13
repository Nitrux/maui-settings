#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class AutostartController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(QStringList commands READ commands NOTIFY commandsChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit AutostartController(QObject *parent = nullptr);

    QString configPath() const;
    bool available() const;
    QStringList commands() const;
    QString errorMessage() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();
    Q_INVOKABLE bool addCommand(const QString &command);
    Q_INVOKABLE bool updateCommand(int index, const QString &command);
    Q_INVOKABLE bool removeCommand(int index);
    Q_INVOKABLE void clearError();

Q_SIGNALS:
    void commandsChanged();
    void errorMessageChanged();

private:
    bool validCommand(const QString &command) const;
    void setErrorMessage(const QString &message);

    QString m_configPath;
    bool m_available = false;
    QStringList m_commands;
    QString m_errorMessage;
};
