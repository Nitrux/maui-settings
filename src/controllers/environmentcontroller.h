#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class EnvironmentController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(QVariantList variables READ variables NOTIFY variablesChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit EnvironmentController(QObject *parent = nullptr);

    QString configPath() const;
    bool available() const;
    QVariantList variables() const;
    QString errorMessage() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();
    Q_INVOKABLE bool addVariable(const QString &variable, const QString &value);
    Q_INVOKABLE bool updateVariable(int index, const QString &variable, const QString &value);
    Q_INVOKABLE bool removeVariable(int index);
    Q_INVOKABLE void clearError();

Q_SIGNALS:
    void variablesChanged();
    void errorMessageChanged();

private:
    bool validVariable(const QString &variable, const QString &value, int ignoredIndex = -1) const;
    void setErrorMessage(const QString &message);

    QString m_configPath;
    bool m_available = false;
    QVariantList m_variables;
    QString m_errorMessage;
};
