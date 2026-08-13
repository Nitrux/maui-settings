#pragma once

#include <QStringList>
#include <QVariantList>

namespace HyprlandDevices
{
QVariantList parse(const QStringList &lines);
QStringList append(QStringList lines, const QString &name, double sensitivity);
QStringList update(QStringList lines, int index, const QString &name, double sensitivity);
QStringList remove(QStringList lines, int index);
}
