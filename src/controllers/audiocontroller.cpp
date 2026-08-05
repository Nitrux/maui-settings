#include "audiocontroller.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QProcess>
#include <QSettings>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QtMath>

AudioController::AudioController(QObject *parent)
    : QObject(parent)
{
    m_hiddenDevices = QSettings().value(QStringLiteral("audio/hiddenDevices")).toStringList();
    refresh();
    m_refreshTimer.setInterval(1000);
    connect(&m_refreshTimer, &QTimer::timeout, this, &AudioController::refresh);
    m_refreshTimer.start();
}

QVariantList AudioController::sinks() const { return m_sinks; }
QVariantList AudioController::sources() const { return m_sources; }
QVariantList AudioController::playbackStreams() const { return m_playbackStreams; }
QVariantList AudioController::captureStreams() const { return m_captureStreams; }
QStringList AudioController::hiddenDevices() const { return m_hiddenDevices; }

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
            node[QStringLiteral("hidden")] = m_hiddenDevices.contains(node.value(QStringLiteral("name")).toString());

            if (mediaClass.startsWith(QStringLiteral("Audio/Sink")))
                sinks << node;
            else if (mediaClass.startsWith(QStringLiteral("Audio/Source")))
                sources << node;
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

void AudioController::setDeviceHidden(const QString &name, bool hidden)
{
    if (name.isEmpty())
        return;

    if (hidden && !m_hiddenDevices.contains(name))
        m_hiddenDevices.append(name);
    else if (!hidden)
        m_hiddenDevices.removeAll(name);
    else
        return;

    QSettings().setValue(QStringLiteral("audio/hiddenDevices"), m_hiddenDevices);
    Q_EMIT hiddenDevicesChanged();
    refresh();
}
