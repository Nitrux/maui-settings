#include "tomainfo.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>
#include <QtGlobal>

namespace {
QString defaultPicturesPath()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    return path.isEmpty() ? QDir::homePath() + QStringLiteral("/Pictures") : path;
}

QString defaultRecordingsPath()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    return path.isEmpty() ? QDir::homePath() + QStringLiteral("/Videos") : path;
}

QString normalizedPath(QString value, const QString &fallback)
{
    value = value.trimmed();
    if (value.startsWith(QStringLiteral("file://")))
        value = QUrl(value).toLocalFile();
    if (value.isEmpty())
        value = fallback;
    if (value == QStringLiteral("~"))
        value = QDir::homePath();
    else if (value.startsWith(QStringLiteral("~/")))
        value = QDir::homePath() + value.mid(1);
    if (!QFileInfo(value).isAbsolute())
        value = QDir::homePath() + QLatin1Char('/') + value;
    return QDir::cleanPath(value);
}

QString configFilePath()
{
    QString directory = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (directory.isEmpty())
        directory = QDir::homePath() + QStringLiteral("/.config");
    return QDir(directory).filePath(QStringLiteral("toma/toma.conf"));
}

QString normalizedChoice(const QString &value, const QStringList &choices, const QString &fallback)
{
    const QString normalized = value.trimmed().toLower();
    return choices.contains(normalized) ? normalized : fallback;
}
}

TomaInfo::TomaInfo(QObject *parent)
    : QObject(parent)
    , m_configPath(configFilePath())
    , m_available(!QStandardPaths::findExecutable(QStringLiteral("toma")).isEmpty())
{
    load();
}

QString TomaInfo::configPath() const { return m_configPath; }
bool TomaInfo::available() const { return m_available; }
QString TomaInfo::screenshotsPath() const { return m_screenshotsPath; }
QString TomaInfo::recordingsPath() const { return m_recordingsPath; }
QString TomaInfo::recordingFormat() const { return m_recordingFormat; }
QString TomaInfo::recordingPreset() const { return m_recordingPreset; }
int TomaInfo::recordingFramerate() const { return m_recordingFramerate; }
int TomaInfo::recordingCountdown() const { return m_recordingCountdown; }
bool TomaInfo::audioEnabled() const { return m_audioEnabled; }
QString TomaInfo::audioDevice() const { return m_audioDevice; }

void TomaInfo::setChanged() { Q_EMIT settingsChanged(); }

void TomaInfo::setDefaults()
{
    m_screenshotsPath = defaultPicturesPath();
    m_recordingsPath = defaultRecordingsPath();
    m_recordingFormat = QStringLiteral("mp4");
    m_recordingPreset = QStringLiteral("balanced");
    m_recordingFramerate = 60;
    m_recordingCountdown = 0;
    m_audioEnabled = false;
    m_audioDevice = QStringLiteral("default");
}

void TomaInfo::setScreenshotsPath(const QString &value)
{
    const QString normalized = normalizedPath(value, defaultPicturesPath());
    if (m_screenshotsPath == normalized)
        return;
    m_screenshotsPath = normalized;
    setChanged();
}

void TomaInfo::setRecordingsPath(const QString &value)
{
    const QString normalized = normalizedPath(value, defaultRecordingsPath());
    if (m_recordingsPath == normalized)
        return;
    m_recordingsPath = normalized;
    setChanged();
}

void TomaInfo::setRecordingFormat(const QString &value)
{
    const QString normalized = normalizedChoice(value, {QStringLiteral("mp4"), QStringLiteral("mkv")}, QStringLiteral("mp4"));
    if (m_recordingFormat == normalized)
        return;
    m_recordingFormat = normalized;
    setChanged();
}

void TomaInfo::setRecordingPreset(const QString &value)
{
    const QString normalized = normalizedChoice(value, {QStringLiteral("low"), QStringLiteral("balanced"), QStringLiteral("high")}, QStringLiteral("balanced"));
    if (m_recordingPreset == normalized)
        return;
    m_recordingPreset = normalized;
    setChanged();
}

void TomaInfo::setRecordingFramerate(int value)
{
    value = qBound(15, value, 360);
    if (m_recordingFramerate == value)
        return;
    m_recordingFramerate = value;
    setChanged();
}

void TomaInfo::setRecordingCountdown(int value)
{
    value = qBound(0, value, 10);
    if (m_recordingCountdown == value)
        return;
    m_recordingCountdown = value;
    setChanged();
}

void TomaInfo::setAudioEnabled(bool value)
{
    if (m_audioEnabled == value)
        return;
    m_audioEnabled = value;
    setChanged();
}

void TomaInfo::setAudioDevice(const QString &value)
{
    const QString normalized = value.trimmed().isEmpty() ? QStringLiteral("default") : value.trimmed();
    if (m_audioDevice == normalized)
        return;
    m_audioDevice = normalized;
    setChanged();
}

void TomaInfo::reload()
{
    load();
}

void TomaInfo::load()
{
    setDefaults();
    const QSettings settings(m_configPath, QSettings::IniFormat);
    m_screenshotsPath = normalizedPath(settings.value(QStringLiteral("Paths/screenshots"), m_screenshotsPath).toString(), m_screenshotsPath);
    m_recordingsPath = normalizedPath(settings.value(QStringLiteral("Paths/recordings"), m_recordingsPath).toString(), m_recordingsPath);
    m_recordingFormat = normalizedChoice(settings.value(QStringLiteral("Recording/format"), m_recordingFormat).toString(), {QStringLiteral("mp4"), QStringLiteral("mkv")}, QStringLiteral("mp4"));
    m_recordingPreset = normalizedChoice(settings.value(QStringLiteral("Recording/preset"), m_recordingPreset).toString(), {QStringLiteral("low"), QStringLiteral("balanced"), QStringLiteral("high")}, QStringLiteral("balanced"));
    m_recordingFramerate = qBound(15, settings.value(QStringLiteral("Recording/framerate"), m_recordingFramerate).toInt(), 360);
    m_recordingCountdown = qBound(0, settings.value(QStringLiteral("Recording/countdown"), m_recordingCountdown).toInt(), 10);
    m_audioEnabled = settings.value(QStringLiteral("Audio/enabled"), m_audioEnabled).toBool();
    m_audioDevice = settings.value(QStringLiteral("Audio/device"), m_audioDevice).toString().trimmed();
    if (m_audioDevice.isEmpty())
        m_audioDevice = QStringLiteral("default");
    setChanged();
}

bool TomaInfo::save()
{
    QDir().mkpath(QFileInfo(m_configPath).absolutePath());
    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("Paths/screenshots"), m_screenshotsPath);
    settings.setValue(QStringLiteral("Paths/recordings"), m_recordingsPath);
    settings.setValue(QStringLiteral("Recording/format"), m_recordingFormat);
    settings.setValue(QStringLiteral("Recording/preset"), m_recordingPreset);
    settings.setValue(QStringLiteral("Recording/framerate"), m_recordingFramerate);
    settings.setValue(QStringLiteral("Recording/countdown"), m_recordingCountdown);
    settings.setValue(QStringLiteral("Audio/enabled"), m_audioEnabled);
    settings.setValue(QStringLiteral("Audio/device"), m_audioDevice);
    settings.sync();
    return settings.status() == QSettings::NoError;
}

void TomaInfo::reset()
{
    setDefaults();
    setChanged();
}
