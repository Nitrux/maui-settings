#pragma once

#include <QObject>
#include <QString>

class TomaInfo final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(QString screenshotsPath READ screenshotsPath WRITE setScreenshotsPath NOTIFY settingsChanged)
    Q_PROPERTY(QString recordingsPath READ recordingsPath WRITE setRecordingsPath NOTIFY settingsChanged)
    Q_PROPERTY(QString recordingFormat READ recordingFormat WRITE setRecordingFormat NOTIFY settingsChanged)
    Q_PROPERTY(QString recordingPreset READ recordingPreset WRITE setRecordingPreset NOTIFY settingsChanged)
    Q_PROPERTY(int recordingFramerate READ recordingFramerate WRITE setRecordingFramerate NOTIFY settingsChanged)
    Q_PROPERTY(int recordingCountdown READ recordingCountdown WRITE setRecordingCountdown NOTIFY settingsChanged)
    Q_PROPERTY(bool audioEnabled READ audioEnabled WRITE setAudioEnabled NOTIFY settingsChanged)
    Q_PROPERTY(QString audioDevice READ audioDevice WRITE setAudioDevice NOTIFY settingsChanged)

public:
    explicit TomaInfo(QObject *parent = nullptr);

    QString configPath() const;
    bool available() const;
    QString screenshotsPath() const;
    QString recordingsPath() const;
    QString recordingFormat() const;
    QString recordingPreset() const;
    int recordingFramerate() const;
    int recordingCountdown() const;
    bool audioEnabled() const;
    QString audioDevice() const;

    void setScreenshotsPath(const QString &value);
    void setRecordingsPath(const QString &value);
    void setRecordingFormat(const QString &value);
    void setRecordingPreset(const QString &value);
    void setRecordingFramerate(int value);
    void setRecordingCountdown(int value);
    void setAudioEnabled(bool value);
    void setAudioDevice(const QString &value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();
    Q_INVOKABLE void reset();

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    void setDefaults();
    void setChanged();

    QString m_configPath;
    bool m_available = false;
    QString m_screenshotsPath;
    QString m_recordingsPath;
    QString m_recordingFormat = QStringLiteral("mp4");
    QString m_recordingPreset = QStringLiteral("balanced");
    int m_recordingFramerate = 60;
    int m_recordingCountdown = 0;
    bool m_audioEnabled = false;
    QString m_audioDevice = QStringLiteral("default");
};
