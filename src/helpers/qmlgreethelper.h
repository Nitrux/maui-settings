#pragma once

#include <KAuth/ActionReply>

#include <QObject>
#include <QVariantMap>

class QmlGreetHelper : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    KAuth::ActionReply save(const QVariantMap &arguments);
    KAuth::ActionReply copyKdeGlobals(const QVariantMap &arguments);
};
