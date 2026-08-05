#pragma once

#include <QObject>
#include <QJsonObject>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>

class AudioController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList sinks READ sinks NOTIFY sinksChanged)
    Q_PROPERTY(QVariantList sources READ sources NOTIFY sourcesChanged)
    Q_PROPERTY(QVariantList playbackStreams READ playbackStreams NOTIFY playbackStreamsChanged)
    Q_PROPERTY(QVariantList captureStreams READ captureStreams NOTIFY captureStreamsChanged)

public:
    explicit AudioController(QObject *parent = nullptr);

    QVariantList sinks() const;
    QVariantList sources() const;
    QVariantList playbackStreams() const;
    QVariantList captureStreams() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void setVolume(quint32 index, qreal volume);
    Q_INVOKABLE void setMuted(quint32 index, bool muted);
    Q_INVOKABLE void setSourceMuted(quint32 index, bool muted);
    Q_INVOKABLE void setDefault(quint32 index);

Q_SIGNALS:
    void sinksChanged();
    void sourcesChanged();
    void playbackStreamsChanged();
    void captureStreamsChanged();

private:
    static QString run(const QStringList &arguments);
    static bool runCommand(const QStringList &arguments);
    static bool runCommandWithExecutable(const QString &name, const QStringList &arguments);
    static QString runPactl(const QStringList &arguments);
    static QString runPactlSourceMute(const QString &target);
    static QString pactlSourceIndex(const QString &name);
    static QVariantMap nodeSnapshot(const QJsonObject &object, const QString &defaultSink, const QString &defaultSource);
    static void applyVolume(QVariantMap &node);
    static QString nodeName(const QString &inspectOutput);

    QVariantList m_sinks;
    QVariantList m_sources;
    QVariantList m_playbackStreams;
    QVariantList m_captureStreams;
};
