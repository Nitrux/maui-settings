// Copyright 2026 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class MimeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList associations READ associations NOTIFY associationsChanged)
    Q_PROPERTY(QString filePath READ filePath CONSTANT)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit MimeController(QObject *parent = nullptr);

    QVariantList associations() const;
    QString filePath() const;
    QString errorMessage() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool setAssociation(const QString &mimeType, const QString &application);
    Q_INVOKABLE bool removeAssociation(const QString &mimeType);
    Q_INVOKABLE void clearError();

Q_SIGNALS:
    void associationsChanged();
    void errorMessageChanged();

private:
    bool writeAssociation(const QString &mimeType, const QString &application, bool remove);
    void setErrorMessage(const QString &message);

    QVariantList m_associations;
    QString m_filePath;
    QString m_errorMessage;
};
