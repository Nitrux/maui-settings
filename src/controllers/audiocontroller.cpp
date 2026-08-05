#include "audiocontroller.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QtMath>

AudioController::AudioController(QObject *parent)
    : QObject(parent)
{
    refresh();
}

QVariantList AudioController::sinks() const { return m_sinks; }
QVariantList AudioController::sources() const { return m_sources; }
QVariantList AudioController::playbackStreams() const { return m_playbackStreams; }
QVariantList AudioController::captureStreams() const { return m_captureStreams; }

QString AudioController::run(const QStringList &arguments)
{
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("wpctl"));
    if (executable.isEmpty())
        return {};

    QProcess process;
    process.start(executable, arguments);
    if (!process.waitForFinished(2500) || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        return {};

    return QString::fromUtf8(process.readAllStandardOutput());
}

bool AudioController::runCommandWithExecutable(const QString &name, const QStringList &arguments)
{
    const QString executable = QStandardPaths::findExecutable(name);
    if (executable.isEmpty())
        return false;

    QProcess process;
    process.start(executable, arguments);
    return process.waitForFinished(2500) && process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

bool AudioController::runCommand(const QStringList &arguments)
{
    return runCommandWithExecutable(QStringLiteral("wpctl"), arguments);
}

QString AudioController::runPactl(const QStringList &arguments)
{
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("pactl"));
    if (executable.isEmpty())
        return {};

    QProcess process;
    process.start(executable, arguments);
    if (!process.waitForFinished(2500) || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        return {};

    return QString::fromUtf8(process.readAllStandardOutput());
}

bool AudioController::deviceAvailable(const QString &name, const QString &deviceType)
{
    const QString output = runPactl({QStringLiteral("list"), deviceType});
    if (output.isEmpty())
        return true;

    const QString deviceHeader = QStringLiteral("Name: %1").arg(name);
    const qsizetype sourceStart = output.indexOf(deviceHeader);
    if (sourceStart < 0)
        return true;

    qsizetype sourceEnd = output.indexOf(QStringLiteral("\n%1 #").arg(deviceType == QStringLiteral("sinks") ? QStringLiteral("Sink") : QStringLiteral("Source")), sourceStart + deviceHeader.size());
    if (sourceEnd < 0)
        sourceEnd = output.size();
    const QString sourceBlock = output.mid(sourceStart, sourceEnd - sourceStart);
    const qsizetype portsStart = sourceBlock.indexOf(QRegularExpression(QStringLiteral("\n\\s*Ports:")));
    if (portsStart < 0)
        return true;

    const qsizetype portsEnd = sourceBlock.indexOf(QRegularExpression(QStringLiteral("\n\\s*Formats:")), portsStart);
    const QString ports = sourceBlock.mid(portsStart, portsEnd < 0 ? sourceBlock.size() - portsStart : portsEnd - portsStart);
    const bool hasAvailablePort = ports.contains(QRegularExpression(QStringLiteral("(?:,\\s*available\\)|available\\s*:\\s*yes)")));
    const bool hasUnavailablePort = ports.contains(QRegularExpression(QStringLiteral("(?:not\\s+available|unavailable|available\\s*:\\s*no)")));
    if (hasAvailablePort)
        return true;
    if (hasUnavailablePort)
        return false;
    return true;
}

QString AudioController::nodeName(const QString &inspectOutput)
{
    static const QRegularExpression expression(QStringLiteral(R"re(node\.name\s*=\s*"([^"]+)")re"));
    const auto match = expression.match(inspectOutput);
    return match.hasMatch() ? match.captured(1) : QString();
}

void AudioController::applyVolume(QVariantMap &node)
{
    const quint32 index = node.value(QStringLiteral("index")).toUInt();
    const QString output = run({QStringLiteral("get-volume"), QString::number(index)});
    static const QRegularExpression expression(QStringLiteral(R"re(Volume:\s*([0-9]*\.?[0-9]+))re"));
    const auto match = expression.match(output);

    if (match.hasMatch())
        node[QStringLiteral("volume")] = qRound(match.captured(1).toDouble() * 100.0);

    const bool muted = output.contains(QStringLiteral("MUTED"), Qt::CaseInsensitive);
    node[QStringLiteral("muted")] = muted;

}

QVariantMap AudioController::nodeSnapshot(const QJsonObject &object, const QString &defaultSink, const QString &defaultSource)
{
    const QJsonObject info = object.value(QStringLiteral("info")).toObject();
    const QJsonObject props = info.value(QStringLiteral("props")).toObject();
    const QString mediaClass = props.value(QStringLiteral("media.class")).toString();
    const QString name = props.value(QStringLiteral("node.name")).toString();
    const QString nick = props.value(QStringLiteral("node.nick")).toString();
    const QString description = props.value(QStringLiteral("node.description")).toString();

    QVariantMap node;
    node[QStringLiteral("index")] = object.value(QStringLiteral("id")).toInt();
    node[QStringLiteral("name")] = name.isEmpty() ? nick : name;
    node[QStringLiteral("description")] = description.isEmpty() ? (nick.isEmpty() ? name : nick) : description;
    node[QStringLiteral("volume")] = 100;
    node[QStringLiteral("muted")] = false;
    node[QStringLiteral("hasVolume")] = true;
    node[QStringLiteral("volumeWritable")] = true;
    node[QStringLiteral("default")] = (mediaClass.startsWith(QStringLiteral("Audio/Sink")) && name == defaultSink)
        || (mediaClass.startsWith(QStringLiteral("Audio/Source")) && name == defaultSource);
    node[QStringLiteral("mediaClass")] = mediaClass;
    applyVolume(node);
    return node;
}

void AudioController::refresh()
{
    const QString dump = [] {
        const QString executable = QStandardPaths::findExecutable(QStringLiteral("pw-dump"));
        if (executable.isEmpty())
            return QString();
        QProcess process;
        process.start(executable, {});
        if (!process.waitForFinished(3500) || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
            return QString();
        return QString::fromUtf8(process.readAllStandardOutput());
    }();

    QVariantList sinks;
    QVariantList sources;
    QVariantList playbackStreams;
    QVariantList captureStreams;
    const QString defaultSink = nodeName(run({QStringLiteral("inspect"), QStringLiteral("@DEFAULT_AUDIO_SINK@")}));
    const QString defaultSource = nodeName(run({QStringLiteral("inspect"), QStringLiteral("@DEFAULT_AUDIO_SOURCE@")}));

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(dump.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError && document.isArray()) {
        for (const QJsonValue &value : document.array()) {
            if (!value.isObject() || value.toObject().value(QStringLiteral("type")).toString() != QStringLiteral("PipeWire:Interface:Node"))
                continue;
            QVariantMap node = nodeSnapshot(value.toObject(), defaultSink, defaultSource);
            const QString mediaClass = node.value(QStringLiteral("mediaClass")).toString();

            if (mediaClass.startsWith(QStringLiteral("Audio/Sink"))) {
                node[QStringLiteral("available")] = deviceAvailable(node.value(QStringLiteral("name")).toString(), QStringLiteral("sinks"));
                sinks << node;
            }
            else if (mediaClass.startsWith(QStringLiteral("Audio/Source"))) {
                if (deviceAvailable(node.value(QStringLiteral("name")).toString(), QStringLiteral("sources")))
                    sources << node;
            }
            else if (mediaClass.contains(QStringLiteral("Stream/Output/Audio")))
                playbackStreams << node;
            else if (mediaClass.contains(QStringLiteral("Stream/Input/Audio")))
                captureStreams << node;
        }
    }

    m_sinks = sinks;
    m_sources = sources;
    m_playbackStreams = playbackStreams;
    m_captureStreams = captureStreams;
    Q_EMIT sinksChanged();
    Q_EMIT sourcesChanged();
    Q_EMIT playbackStreamsChanged();
    Q_EMIT captureStreamsChanged();
}

void AudioController::setVolume(quint32 index, qreal volume)
{
    const qreal normalized = qBound<qreal>(0.0, volume / 100.0, 1.5);
    runCommand({QStringLiteral("set-volume"), QString::number(index), QString::number(normalized, 'f', 2)});
}

void AudioController::setMuted(quint32 index, bool muted)
{
    if (runCommand({QStringLiteral("set-mute"), QString::number(index), muted ? QStringLiteral("1") : QStringLiteral("0")}))
        refresh();
}

void AudioController::setSourceMuted(quint32 index, bool muted)
{
    const bool ok = runCommand({QStringLiteral("set-mute"), QString::number(index), muted ? QStringLiteral("1") : QStringLiteral("0")});
    if (ok)
        refresh();
}

void AudioController::setDefault(quint32 index)
{
    if (runCommand({QStringLiteral("set-default"), QString::number(index)}))
        refresh();
}
