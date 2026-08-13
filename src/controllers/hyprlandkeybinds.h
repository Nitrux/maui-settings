#pragma once

#include <QStringList>
#include <QVariantList>

namespace HyprlandKeybinds
{
QVariantList parse(const QStringList &lines);
QStringList append(QStringList lines, const QString &key, const QString &command);
QStringList update(QStringList lines, int index, const QString &key, const QString &command, const QString &options = {}, const QString &keyExpression = {}, const QString &actionExpression = {});
QStringList remove(QStringList lines, int index);
}
