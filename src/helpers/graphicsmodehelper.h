#pragma once

#include <KAuth/ActionReply>

#include <QObject>
#include <QVariantMap>

class GraphicsModeHelper final : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    KAuth::ActionReply set(const QVariantMap &arguments);
};
