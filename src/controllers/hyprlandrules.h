#pragma once

#include <QString>
#include <QStringList>
#include <QVariantList>

namespace HyprlandRules
{
QVariantList parse(const QStringList &lines);
QStringList append(QStringList lines, const QString &type, const QString &name,
                   const QString &matchKey, const QString &matchValue,
                   const QString &action);
QStringList update(QStringList lines, int index, const QString &name,
                   const QString &matchKey, const QString &matchValue,
                   const QString &action);
QStringList remove(QStringList lines, int index);
}
