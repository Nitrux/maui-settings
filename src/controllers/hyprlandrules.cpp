#include "hyprlandrules.h"

#include <QList>
#include <QRegularExpression>
#include <QVariantMap>

namespace
{
struct RuleBlock
{
    int start = -1;
    int end = -1;
    QString type;
};

int braceDelta(const QString &line)
{
    bool quoted = false;
    bool escaped = false;
    int delta = 0;

    for (int i = 0; i < line.size(); ++i)
    {
        const QChar character = line.at(i);
        if (!quoted && character == QLatin1Char('-') && i + 1 < line.size()
            && line.at(i + 1) == QLatin1Char('-'))
            break;
        if (character == QLatin1Char('"') && !escaped)
            quoted = !quoted;
        if (!quoted)
        {
            if (character == QLatin1Char('{'))
                ++delta;
            else if (character == QLatin1Char('}'))
                --delta;
        }
        escaped = character == QLatin1Char('\\') && !escaped;
        if (character != QLatin1Char('\\'))
            escaped = false;
    }
    return delta;
}

QList<RuleBlock> blocks(const QStringList &lines)
{
    QList<RuleBlock> result;
    const QRegularExpression expression(
        QStringLiteral("^\\s*hl\\.(window_rule|layer_rule)\\s*\\(\\s*\\{"));

    for (int start = 0; start < lines.size(); ++start)
    {
        const auto match = expression.match(lines.at(start));
        if (!match.hasMatch())
            continue;

        int depth = 0;
        for (int line = start; line < lines.size(); ++line)
        {
            depth += braceDelta(lines.at(line));
            if (depth == 0)
            {
                result.append({start, line,
                               match.captured(1) == QLatin1String("window_rule")
                                   ? QStringLiteral("window")
                                   : QStringLiteral("layer")});
                start = line;
                break;
            }
        }
    }
    return result;
}

QString luaString(const QString &value)
{
    QString escaped = value;
    escaped.replace(QChar(92), QString(2, QChar(92)));
    escaped.replace(QChar(34), QString(QChar(92)) + QChar(34));
    return QStringLiteral("\"%1\"").arg(escaped);
}

QVariantMap parseBlock(const QStringList &lines, const RuleBlock &block)
{
    const QString source = lines.mid(block.start, block.end - block.start + 1)
        .join(QLatin1Char('\n'));
    QVariantMap rule;
    rule.insert(QStringLiteral("type"), block.type);

    const auto name = QRegularExpression(
        QStringLiteral("\\bname\\s*=\\s*\"([^\"]*)\"")).match(source);
    rule.insert(QStringLiteral("name"), name.hasMatch() ? name.captured(1) : QString());

    const auto match = QRegularExpression(
        QStringLiteral("match\\s*=\\s*\\{[\\s\\S]*?\\b(class|title|namespace|appid)\\s*=\\s*\"([^\"]*)\""))
        .match(source);
    rule.insert(QStringLiteral("matchKey"), match.hasMatch()
        ? match.captured(1)
        : (block.type == QLatin1String("layer") ? QStringLiteral("namespace") : QStringLiteral("class")));
    rule.insert(QStringLiteral("matchValue"), match.hasMatch() ? match.captured(2) : QString());

    QString action;
    const QStringList actionKeys = {
        QStringLiteral("float"), QStringLiteral("center"), QStringLiteral("blur"),
        QStringLiteral("blur_popups"), QStringLiteral("stay_focused"),
        QStringLiteral("focus_on_activate"), QStringLiteral("modal")};
    int sourceDepth = 0;
    for (const QString &line : source.split(QLatin1Char(10)))
    {
        const int depthBeforeLine = sourceDepth;
        sourceDepth += braceDelta(line);
        if (depthBeforeLine != 1 || line.contains(QStringLiteral("match")))
            continue;
        for (const QString &key : actionKeys)
        {
            if (QRegularExpression(QStringLiteral("\\b%1\\s*=\\s*true").arg(QRegularExpression::escape(key))).match(line).hasMatch())
            {
                action = key;
                break;
            }
        }
        if (!action.isEmpty())
            break;
    }
    if (action.isEmpty() && source.contains(QStringLiteral("suppress_event = \"maximize\"")))
        action = QStringLiteral("suppress_event");
    else if (action.isEmpty() && source.contains(QStringLiteral("ignore_alpha = 0.4")))
        action = QStringLiteral("ignore_alpha");
    rule.insert(QStringLiteral("action"), action);

    QStringList details;
    const QStringList keys = {
        QStringLiteral("float"), QStringLiteral("center"), QStringLiteral("blur"),
        QStringLiteral("blur_popups"), QStringLiteral("stay_focused"),
        QStringLiteral("focus_on_activate"), QStringLiteral("modal"),
        QStringLiteral("suppress_event"), QStringLiteral("size"),
        QStringLiteral("min_size"), QStringLiteral("ignore_alpha")};
    for (const QString &key : keys)
    {
        const auto value = QRegularExpression(
            QStringLiteral("\\b%1\\s*=\\s*([^,}\\n]+)")
                .arg(QRegularExpression::escape(key))).match(source);
        if (value.hasMatch())
            details.append(QStringLiteral("%1 = %2").arg(key, value.captured(1).trimmed()));
    }
    rule.insert(QStringLiteral("details"), details.join(QStringLiteral(", ")));
    return rule;
}

QString build(const QString &type, const QString &name, const QString &matchKey,
              const QString &matchValue, const QString &action)
{
    const QString function = type == QLatin1String("layer")
        ? QStringLiteral("layer_rule") : QStringLiteral("window_rule");
    QStringList lines = {
        QStringLiteral("hl.%1({").arg(function),
        QStringLiteral("    name = %1,").arg(luaString(name)),
        QStringLiteral("    match = { %1 = %2 },").arg(matchKey, luaString(matchValue)),
    };

    if (action == QLatin1String("float") || action == QLatin1String("center")
        || action == QLatin1String("blur") || action == QLatin1String("blur_popups")
        || action == QLatin1String("stay_focused")
        || action == QLatin1String("focus_on_activate") || action == QLatin1String("modal"))
        lines.append(QStringLiteral("    %1 = true,").arg(action));
    else if (action == QLatin1String("suppress_event"))
        lines.append(QStringLiteral("    suppress_event = \"maximize\","));
    else if (action == QLatin1String("ignore_alpha"))
        lines.append(QStringLiteral("    ignore_alpha = 0.4,"));

    lines.append(QStringLiteral("})"));
    return lines.join(QLatin1Char('\n'));
}
}

QVariantList HyprlandRules::parse(const QStringList &lines)
{
    QVariantList result;
    for (const RuleBlock &block : blocks(lines))
        result.append(parseBlock(lines, block));
    return result;
}

QStringList HyprlandRules::append(QStringList lines, const QString &type,
                                  const QString &name, const QString &matchKey,
                                  const QString &matchValue, const QString &action)
{
    while (!lines.isEmpty() && lines.constLast().isEmpty())
        lines.removeLast();
    if (!lines.isEmpty())
        lines.append(QString());
    lines += build(type, name, matchKey, matchValue, action).split(QLatin1Char('\n'));
    lines.append(QString());
    return lines;
}

QStringList HyprlandRules::update(QStringList lines, int index, const QString &name,
                                  const QString &matchKey, const QString &matchValue,
                                  const QString &action)
{
    const QList<RuleBlock> ruleBlocks = blocks(lines);
    if (index < 0 || index >= ruleBlocks.size())
        return {};

    const RuleBlock block = ruleBlocks.at(index);
    QStringList replacement = lines.mid(block.start, block.end - block.start + 1);
    const QString source = replacement.join(QLatin1Char(10));

    const QRegularExpression nameExpression(QStringLiteral("(\\bname\\s*=\\s*)\"[^\"]*\""));
    bool nameReplaced = false;
    for (int line = 0; line < replacement.size(); ++line)
    {
        auto nameMatch = nameExpression.match(replacement.at(line));
        if (nameMatch.hasMatch())
        {
            replacement[line].replace(nameMatch.capturedStart(0), nameMatch.capturedLength(0),
                                      QStringLiteral("name = %1").arg(luaString(name)));
            nameReplaced = true;
            break;
        }
    }
    if (!nameReplaced)
        replacement.insert(1, QStringLiteral("    name = %1,").arg(luaString(name)));

    const QRegularExpression matchExpression(QStringLiteral("\\b(class|title|namespace|appid)\\s*=\\s*\"[^\"]*\""));
    for (int line = 0; line < replacement.size(); ++line)
    {
        auto match = matchExpression.match(replacement.at(line));
        if (match.hasMatch())
        {
            replacement[line].replace(match.capturedStart(0), match.capturedLength(0),
                                      QStringLiteral("%1 = %2").arg(matchKey, luaString(matchValue)));
            break;
        }
    }

    const QRegularExpression actionExpression(QStringLiteral("^\\s*(float|center|blur|blur_popups|stay_focused|focus_on_activate|modal|suppress_event|ignore_alpha)\\s*="));
    QList<int> actionLines;
    int replacementDepth = 0;
    for (int line = 0; line < replacement.size(); ++line)
    {
        const int depthBeforeLine = replacementDepth;
        replacementDepth += braceDelta(replacement.at(line));
        if (depthBeforeLine == 1 && !replacement.at(line).contains(QStringLiteral("match")))
        {
            if (actionExpression.match(replacement.at(line)).hasMatch())
                actionLines.append(line);
        }
    }
    for (int line = actionLines.size() - 1; line >= 0; --line)
        replacement.removeAt(actionLines.at(line));

    QString actionLine;
    if (action == QLatin1String("float") || action == QLatin1String("center")
        || action == QLatin1String("blur") || action == QLatin1String("blur_popups")
        || action == QLatin1String("stay_focused") || action == QLatin1String("focus_on_activate")
        || action == QLatin1String("modal"))
        actionLine = QStringLiteral("    %1 = true,").arg(action);
    else if (action == QLatin1String("suppress_event"))
        actionLine = QStringLiteral("    suppress_event = \"maximize\",");
    else if (action == QLatin1String("ignore_alpha"))
        actionLine = QStringLiteral("    ignore_alpha = 0.4,");
    if (!actionLine.isEmpty())
        replacement.insert(replacement.size() - 1, actionLine);

    lines.remove(block.start, block.end - block.start + 1);
    for (int line = 0; line < replacement.size(); ++line)
        lines.insert(block.start + line, replacement.at(line));
    return lines;
}

QStringList HyprlandRules::remove(QStringList lines, int index)
{
    const QList<RuleBlock> ruleBlocks = blocks(lines);
    if (index < 0 || index >= ruleBlocks.size())
        return {};

    const RuleBlock block = ruleBlocks.at(index);
    lines.remove(block.start, block.end - block.start + 1);
    if (block.start < lines.size() && lines.at(block.start).isEmpty()
        && block.start > 0 && lines.at(block.start - 1).isEmpty())
        lines.removeAt(block.start);
    return lines;
}
