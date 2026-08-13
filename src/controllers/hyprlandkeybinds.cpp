#include "hyprlandkeybinds.h"

#include <QRegularExpression>
#include <QVariantMap>

namespace
{
struct BindingBlock
{
    int start = -1;
    int end = -1;
};

bool isEscaped(const QString &source, int index)
{
    int backslashes = 0;
    for (int i = index - 1; i >= 0 && source.at(i) == QLatin1Char('\\'); --i)
        ++backslashes;
    return backslashes % 2 != 0;
}

int callEnd(const QString &source, int opening)
{
    int depth = 0;
    bool quoted = false;

    for (int i = opening; i < source.size(); ++i)
    {
        const QChar character = source.at(i);
        if (character == QLatin1Char('"') && !isEscaped(source, i))
            quoted = !quoted;
        if (quoted)
            continue;
        if (character == QLatin1Char('('))
            ++depth;
        else if (character == QLatin1Char(')') && --depth == 0)
            return i;
    }

    return -1;
}

QList<BindingBlock> blocks(const QStringList &lines)
{
    QList<BindingBlock> result;
    const QRegularExpression expression(QStringLiteral("^\\s*hl\\.bind\\s*\\("));

    for (int line = 0; line < lines.size(); ++line)
    {
        const auto match = expression.match(lines.at(line));
        if (!match.hasMatch())
            continue;

        const int opening = lines.at(line).indexOf(QLatin1Char('('), match.capturedStart());
        if (opening < 0)
            continue;

        QString source;
        int end = line;
        int closing = -1;
        for (; end < lines.size(); ++end)
        {
            if (!source.isEmpty())
                source += QLatin1Char('\n');
            source += lines.at(end);
            const int localOpening = end == line ? opening : source.indexOf(QLatin1Char('('));
            closing = callEnd(source, localOpening);
            if (closing >= 0)
                break;
        }

        if (closing >= 0)
        {
            result.append({line, end});
            line = end;
        }
    }
    return result;
}

QStringList arguments(const QString &source)
{
    const int opening = source.indexOf(QLatin1Char('('));
    const int closing = source.lastIndexOf(QLatin1Char(')'));
    if (opening < 0 || closing <= opening)
        return {};

    const QString body = source.mid(opening + 1, closing - opening - 1);
    QStringList result;
    int start = 0;
    int parentheses = 0;
    int braces = 0;
    int brackets = 0;
    bool quoted = false;

    for (int i = 0; i < body.size(); ++i)
    {
        const QChar character = body.at(i);
        if (character == QLatin1Char('"') && !isEscaped(body, i))
            quoted = !quoted;
        if (quoted)
            continue;

        if (character == QLatin1Char('('))
            ++parentheses;
        else if (character == QLatin1Char(')'))
            --parentheses;
        else if (character == QLatin1Char('{'))
            ++braces;
        else if (character == QLatin1Char('}'))
            --braces;
        else if (character == QLatin1Char('['))
            ++brackets;
        else if (character == QLatin1Char(']'))
            --brackets;
        else if (character == QLatin1Char(',') && parentheses == 0 && braces == 0 && brackets == 0)
        {
            result.append(body.mid(start, i - start).trimmed());
            start = i + 1;
        }
    }
    result.append(body.mid(start).trimmed());
    return result;
}

QString unquote(const QString &value)
{
    const QString trimmed = value.trimmed();
    if (trimmed.size() < 2 || !trimmed.startsWith(QLatin1Char('"'))
        || !trimmed.endsWith(QLatin1Char('"')))
        return trimmed;

    QString result = trimmed.mid(1, trimmed.size() - 2);
    result.replace(QStringLiteral("\\\""), QStringLiteral("\""));
    result.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
    return result;
}

QString displayExpression(const QString &value)
{
    const QString trimmed = value.trimmed();
    const QStringList parts = trimmed.split(QRegularExpression(QStringLiteral("\\s*\\.\\.\\s*")));
    QString display;
    for (const QString &partValue : parts)
    {
        const QString part = partValue.trimmed();
        display += part.startsWith(QLatin1Char(34)) ? unquote(part) : part;
    }
    return display;
}

QString execCommand(const QString &action)
{
    const QString prefix = QStringLiteral("hl.dsp.exec_cmd");
    const int start = action.indexOf(prefix);
    if (start < 0)
        return action.trimmed();

    const int opening = action.indexOf(QLatin1Char('('), start + prefix.size());
    if (opening < 0)
        return action.trimmed();
    const int closing = callEnd(action, opening);
    if (closing < 0)
        return action.trimmed();

    const QStringList args = arguments(action.mid(start, closing - start + 1));
    return args.isEmpty() ? QString() : displayExpression(args.first());
}

QString luaString(const QString &value)
{
    QString escaped = value.trimmed();
    escaped.replace(QChar(92), QString(2, QChar(92)));
    escaped.replace(QChar(34), QString(QChar(92)) + QChar(34));
    escaped.replace(QChar(10), QStringLiteral("\\n"));
    return QStringLiteral("\"%1\"").arg(escaped);
}

QString bindingLine(const QString &key, const QString &command, const QString &indent = {}, const QString &options = {}, const QString &keyExpression = {}, const QString &actionExpression = {})
{
    const QString normalizedCommand = command.trimmed();
    const QString action = actionExpression.trimmed().isEmpty()
        ? (normalizedCommand.startsWith(QStringLiteral("hl.dsp.")) ? normalizedCommand : QStringLiteral("hl.dsp.exec_cmd(%1)").arg(luaString(normalizedCommand)))
        : actionExpression.trimmed();
    const QString suffix = options.trimmed().isEmpty() ? QString() : QStringLiteral(", %1").arg(options.trimmed());
    const QString keySource = keyExpression.trimmed().isEmpty() ? luaString(key) : keyExpression.trimmed();
    return indent + QStringLiteral("hl.bind(%1, %2%3)").arg(keySource, action, suffix);
}

QVariantMap parseBinding(const QStringList &lines, const BindingBlock &block)
{
    const QString source = lines.mid(block.start, block.end - block.start + 1).join(QLatin1Char('\n'));
    const QStringList args = arguments(source);
    const QString keyExpression = args.value(0);
    const QString action = args.value(1);
    const QString options = args.value(2);

    QVariantMap binding;
    binding.insert(QStringLiteral("key"), displayExpression(keyExpression));
    binding.insert(QStringLiteral("keyExpression"), keyExpression);
    binding.insert(QStringLiteral("action"), action);
    const QString command = execCommand(action);
    binding.insert(QStringLiteral("command"), command);
    binding.insert(QStringLiteral("options"), options);
    binding.insert(QStringLiteral("mouse"), options.contains(QRegularExpression(QStringLiteral("\\bmouse\\s*=\\s*true"))));
    binding.insert(QStringLiteral("locked"), options.contains(QRegularExpression(QStringLiteral("\\blocked\\s*=\\s*true"))));
    return binding;
}

QStringList replaceBinding(QStringList lines, int index, const QString &key, const QString &command, const QString &options, const QString &keyExpression, const QString &actionExpression)
{
    const auto bindingBlocks = blocks(lines);
    if (index < 0 || index >= bindingBlocks.size())
        return {};

    const BindingBlock block = bindingBlocks.at(index);
    const QString sourceLine = lines.at(block.start);
    const QString indent = sourceLine.left(sourceLine.indexOf(QRegularExpression(QStringLiteral("\\S"))));
    lines.erase(lines.begin() + block.start, lines.begin() + block.end + 1);
    lines.insert(block.start, bindingLine(key, command, indent, options, keyExpression, actionExpression));
    return lines;
}
}

namespace HyprlandKeybinds
{
QVariantList parse(const QStringList &lines)
{
    QVariantList result;
    for (const BindingBlock &block : blocks(lines))
        result.append(parseBinding(lines, block));
    return result;
}

QStringList append(QStringList lines, const QString &key, const QString &command)
{
    int insertion = lines.size();
    while (insertion > 0 && lines.at(insertion - 1).trimmed().isEmpty())
        --insertion;

    if (blocks(lines).isEmpty())
    {
        lines.insert(insertion++, QString());
        lines.insert(insertion++, QStringLiteral("---------------------"));
        lines.insert(insertion++, QStringLiteral("---- KEYBINDINGS ----"));
        lines.insert(insertion++, QStringLiteral("---------------------"));
    }
    lines.insert(insertion, bindingLine(key, command));
    return lines;
}

QStringList update(QStringList lines, int index, const QString &key, const QString &command, const QString &options, const QString &keyExpression, const QString &actionExpression)
{
    return replaceBinding(lines, index, key, command, options, keyExpression, actionExpression);
}

QStringList remove(QStringList lines, int index)
{
    const auto bindingBlocks = blocks(lines);
    if (index < 0 || index >= bindingBlocks.size())
        return {};

    const BindingBlock block = bindingBlocks.at(index);
    lines.erase(lines.begin() + block.start, lines.begin() + block.end + 1);
    return lines;
}
}
