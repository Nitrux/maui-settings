// Copyright 2026 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <KAuth/ActionReply>

#include <QObject>
#include <QVariantMap>

class QmlGreetHelper : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    KAuth::ActionReply save(const QVariantMap &arguments);
};
