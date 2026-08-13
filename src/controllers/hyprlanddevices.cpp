#include "hyprlanddevices.h"

#include <QRegularExpression>
#include <QVariantMap>

namespace
{
struct DeviceBlock { int start = -1; int end = -1; };

int braceDelta(const QString &line)
{
    bool quoted = false;
    bool escaped = false;
    int delta = 0;
    for (int i = 0; i < line.size(); ++i)
    {
        const QChar character = line.at(i);
        if (character == QLatin1Char(34) && !escaped) quoted = !quoted;
        if (!quoted)
        {
            if (character == QLatin1Char(123)) ++delta;
            else if (character == QLatin1Char(125)) --delta;
        }
        escaped = character == QLatin1Char(92) && !escaped;
        if (character != QLatin1Char(92)) escaped = false;
    }
    return delta;
}

QList<DeviceBlock> blocks(const QStringList &lines)
{
    QList<DeviceBlock> result;
    const QRegularExpression expression(QStringLiteral("^\\s*hl\\.device\\s*\\(\\s*\\{"));
    for (int start = 0; start < lines.size(); ++start)
    {
        if (!expression.match(lines.at(start)).hasMatch()) continue;
        int depth = 0;
        for (int end = start; end < lines.size(); ++end)
        {
            depth += braceDelta(lines.at(end));
            if (depth == 0)
            {
                result.append({start, end});
                start = end;
                break;
            }
        }
    }
    return result;
}

QString luaString(const QString &value)
{
    QString escaped = value.trimmed();
    escaped.replace(QChar(92), QString(2, QChar(92)));
    escaped.replace(QChar(34), QString(QChar(92)) + QChar(34));
    return QStringLiteral("\"%1\"").arg(escaped);
}

QString deviceSource(const QString &name, double sensitivity, const QString &indent = {})
{
    return indent + QStringLiteral("hl.device({ name = %1, sensitivity = %2 })")
        .arg(luaString(name), QString::number(sensitivity, static_cast<char>(102), 2));
}

QVariantMap parseDevice(const QStringList &lines, const DeviceBlock &block)
{
    const QString source = lines.mid(block.start, block.end - block.start + 1).join(QLatin1Char(10));
    const auto nameMatch = QRegularExpression(QStringLiteral("\\bname\\s*=\\s*\"([^\"]*)\"")).match(source);
    const auto sensitivityMatch = QRegularExpression(QStringLiteral("\\bsensitivity\\s*=\\s*(-?\\d+(?:\\.\\d+)?)")).match(source);
    bool ok = false;
    const double sensitivity = sensitivityMatch.hasMatch() ? sensitivityMatch.captured(1).toDouble(&ok) : 0.0;
    QVariantMap device;
    device.insert(QStringLiteral("name"), nameMatch.hasMatch() ? nameMatch.captured(1) : QString());
    device.insert(QStringLiteral("sensitivity"), ok ? sensitivity : 0.0);
    return device;
}

QStringList replaceDevice(QStringList lines, int index, const QString &name, double sensitivity)
{
    const auto deviceBlocks = blocks(lines);
    if (index < 0 || index >= deviceBlocks.size()) return {};
    const DeviceBlock block = deviceBlocks.at(index);
    const QString sourceLine = lines.at(block.start);
    const int first = sourceLine.indexOf(QRegularExpression(QStringLiteral("\\S")));
    const QString indent = first > 0 ? sourceLine.left(first) : QString();
    lines.erase(lines.begin() + block.start, lines.begin() + block.end + 1);
    lines.insert(block.start, deviceSource(name, sensitivity, indent));
    return lines;
}
}

namespace HyprlandDevices
{
QVariantList parse(const QStringList &lines)
{
    QVariantList result;
    for (const DeviceBlock &block : blocks(lines)) result.append(parseDevice(lines, block));
    return result;
}

QStringList append(QStringList lines, const QString &name, double sensitivity)
{
    int insertion = lines.size();
    while (insertion > 0 && lines.at(insertion - 1).trimmed().isEmpty()) --insertion;
    if (!blocks(lines).isEmpty()) lines.insert(insertion++, QString());
    lines.insert(insertion, deviceSource(name, sensitivity));
    return lines;
}

QStringList update(QStringList lines, int index, const QString &name, double sensitivity)
{
    return replaceDevice(lines, index, name, sensitivity);
}

QStringList remove(QStringList lines, int index)
{
    const auto deviceBlocks = blocks(lines);
    if (index < 0 || index >= deviceBlocks.size()) return {};
    const DeviceBlock block = deviceBlocks.at(index);
    lines.erase(lines.begin() + block.start, lines.begin() + block.end + 1);
    return lines;
}
}
