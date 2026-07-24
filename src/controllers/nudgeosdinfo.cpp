#include "nudgeosdinfo.h"

#include <QDir>
#include <QSettings>
#include <QtGlobal>

NudgeOsdInfo::NudgeOsdInfo(QObject *parent)
    : QObject(parent)
    , m_configPath(QDir::homePath() + QStringLiteral("/.config/nudge-osd/nudge-osd.conf"))
{
    load();
}

QString NudgeOsdInfo::configPath() const { return m_configPath; }
QString NudgeOsdInfo::iconMode() const { return m_iconMode; }
int NudgeOsdInfo::osdWidth() const { return m_osdWidth; }
int NudgeOsdInfo::osdHeight() const { return m_osdHeight; }
int NudgeOsdInfo::bottomOffset() const { return m_bottomOffset; }
int NudgeOsdInfo::hideTimeout() const { return m_hideTimeout; }
int NudgeOsdInfo::showAnimationDuration() const { return m_showAnimationDuration; }
int NudgeOsdInfo::hideAnimationDuration() const { return m_hideAnimationDuration; }
int NudgeOsdInfo::volumeStep() const { return m_volumeStep; }
int NudgeOsdInfo::brightnessStep() const { return m_brightnessStep; }

void NudgeOsdInfo::setChanged() { Q_EMIT settingsChanged(); }

void NudgeOsdInfo::setIconMode(const QString &value)
{
    const QString normalized = value.trimmed().toLower() == QLatin1String("emoji") ? QStringLiteral("emoji") : QStringLiteral("system");
    if (m_iconMode == normalized)
        return;
    m_iconMode = normalized;
    setChanged();
}

#define SET_BOUNDED_INTEGER(Name, Member, Minimum, Maximum) \
    void NudgeOsdInfo::set##Name(int value) \
    { \
        value = qBound(Minimum, value, Maximum); \
        if (Member == value) \
            return; \
        Member = value; \
        setChanged(); \
    }

SET_BOUNDED_INTEGER(OsdWidth, m_osdWidth, 160, 800)
SET_BOUNDED_INTEGER(OsdHeight, m_osdHeight, 56, 200)
SET_BOUNDED_INTEGER(BottomOffset, m_bottomOffset, 0, 1000)
SET_BOUNDED_INTEGER(HideTimeout, m_hideTimeout, 250, 10000)
SET_BOUNDED_INTEGER(ShowAnimationDuration, m_showAnimationDuration, 0, 2000)
SET_BOUNDED_INTEGER(HideAnimationDuration, m_hideAnimationDuration, 0, 2000)
SET_BOUNDED_INTEGER(VolumeStep, m_volumeStep, 1, 100)
SET_BOUNDED_INTEGER(BrightnessStep, m_brightnessStep, 1, 100)

#undef SET_BOUNDED_INTEGER

void NudgeOsdInfo::reload()
{
    load();
}

void NudgeOsdInfo::load()
{
    QSettings settings(m_configPath, QSettings::IniFormat);
    m_iconMode = settings.value(QStringLiteral("Appearance/iconMode"), QStringLiteral("system")).toString().trimmed().toLower() == QLatin1String("emoji")
        ? QStringLiteral("emoji")
        : QStringLiteral("system");
    m_osdWidth = qBound(160, settings.value(QStringLiteral("Appearance/width"), 292).toInt(), 800);
    m_osdHeight = qBound(56, settings.value(QStringLiteral("Appearance/height"), 66).toInt(), 200);
    m_bottomOffset = qBound(0, settings.value(QStringLiteral("Position/bottomOffset"), 114).toInt(), 1000);
    m_hideTimeout = qBound(250, settings.value(QStringLiteral("Behavior/hideTimeout"), 2000).toInt(), 10000);
    m_showAnimationDuration = qBound(0, settings.value(QStringLiteral("Behavior/showAnimationDuration"), 200).toInt(), 2000);
    m_hideAnimationDuration = qBound(0, settings.value(QStringLiteral("Behavior/hideAnimationDuration"), 200).toInt(), 2000);
    m_volumeStep = qBound(1, settings.value(QStringLiteral("Controls/volumeStep"), 5).toInt(), 100);
    m_brightnessStep = qBound(1, settings.value(QStringLiteral("Controls/brightnessStep"), 10).toInt(), 100);
    setChanged();
}

bool NudgeOsdInfo::save()
{
    QDir().mkpath(QDir::homePath() + QStringLiteral("/.config/nudge-osd"));
    QSettings settings(m_configPath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("Appearance/iconMode"), m_iconMode);
    settings.setValue(QStringLiteral("Appearance/width"), m_osdWidth);
    settings.setValue(QStringLiteral("Appearance/height"), m_osdHeight);
    settings.setValue(QStringLiteral("Position/bottomOffset"), m_bottomOffset);
    settings.setValue(QStringLiteral("Behavior/hideTimeout"), m_hideTimeout);
    settings.setValue(QStringLiteral("Behavior/showAnimationDuration"), m_showAnimationDuration);
    settings.setValue(QStringLiteral("Behavior/hideAnimationDuration"), m_hideAnimationDuration);
    settings.setValue(QStringLiteral("Controls/volumeStep"), m_volumeStep);
    settings.setValue(QStringLiteral("Controls/brightnessStep"), m_brightnessStep);
    settings.sync();
    return settings.status() == QSettings::NoError;
}
