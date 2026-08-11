#include "hyprsunsetinfo.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QtMath>
#include <QtGlobal>

namespace
{
QString assignmentValue(const QString &line, const QString &key)
{
    const QRegularExpression expression(QStringLiteral(R"(^(%1)\s*=\s*(.*)$)").arg(QRegularExpression::escape(key)));
    const QRegularExpressionMatch match = expression.match(line.trimmed());
    return match.hasMatch() ? match.captured(2).trimmed() : QString();
}

bool parseBool(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    return normalized == QStringLiteral("1")
        || normalized == QStringLiteral("true")
        || normalized == QStringLiteral("yes")
        || normalized == QStringLiteral("on");
}

bool parseTime(const QString &value, int *hour, int *minute)
{
    const QStringList parts = value.trimmed().split(QLatin1Char(58), Qt::KeepEmptyParts);
    if (parts.size() != 2)
        return false;

    bool hourOk = false;
    bool minuteOk = false;
    const int parsedHour = parts.at(0).toInt(&hourOk);
    const int parsedMinute = parts.at(1).toInt(&minuteOk);
    if (!hourOk || !minuteOk || parsedHour < 0 || parsedHour > 23 || parsedMinute < 0 || parsedMinute > 59)
        return false;

    *hour = parsedHour;
    *minute = parsedMinute;
    return true;
}
} // namespace

HyprsunsetInfo::HyprsunsetInfo(QObject *parent)
    : QObject(parent)
    , m_configPath(QDir::homePath() + QStringLiteral("/.config/hypr/hyprsunset.conf"))
{
    load();
}

QString HyprsunsetInfo::configPath() const { return m_configPath; }
bool HyprsunsetInfo::available() const { return m_available; }
bool HyprsunsetInfo::configAvailable() const { return m_configAvailable; }
int HyprsunsetInfo::maxGamma() const { return m_maxGamma; }
int HyprsunsetInfo::daytimeHour() const { return m_daytimeHour; }
int HyprsunsetInfo::daytimeMinute() const { return m_daytimeMinute; }
bool HyprsunsetInfo::daytimeIdentity() const { return m_daytimeIdentity; }
int HyprsunsetInfo::nighttimeHour() const { return m_nighttimeHour; }
int HyprsunsetInfo::nighttimeMinute() const { return m_nighttimeMinute; }
int HyprsunsetInfo::nighttimeTemperature() const { return m_nighttimeTemperature; }
int HyprsunsetInfo::nighttimeGammaPercent() const { return m_nighttimeGammaPercent; }

void HyprsunsetInfo::setChanged()
{
    Q_EMIT settingsChanged();
}

void HyprsunsetInfo::setMaxGamma(int value)
{
    value = qBound(0, value, 200);
    if (m_maxGamma == value)
        return;
    m_maxGamma = value;
    setChanged();
}

void HyprsunsetInfo::setDaytimeHour(int value)
{
    value = qBound(0, value, 23);
    if (m_daytimeHour == value)
        return;
    m_daytimeHour = value;
    setChanged();
}

void HyprsunsetInfo::setDaytimeMinute(int value)
{
    value = qBound(0, value, 59);
    if (m_daytimeMinute == value)
        return;
    m_daytimeMinute = value;
    setChanged();
}

void HyprsunsetInfo::setDaytimeIdentity(bool value)
{
    if (m_daytimeIdentity == value)
        return;
    m_daytimeIdentity = value;
    setChanged();
}

void HyprsunsetInfo::setNighttimeHour(int value)
{
    value = qBound(0, value, 23);
    if (m_nighttimeHour == value)
        return;
    m_nighttimeHour = value;
    setChanged();
}

void HyprsunsetInfo::setNighttimeMinute(int value)
{
    value = qBound(0, value, 59);
    if (m_nighttimeMinute == value)
        return;
    m_nighttimeMinute = value;
    setChanged();
}

void HyprsunsetInfo::setNighttimeTemperature(int value)
{
    value = qBound(1000, value, 10000);
    if (m_nighttimeTemperature == value)
        return;
    m_nighttimeTemperature = value;
    setChanged();
}

void HyprsunsetInfo::setNighttimeGammaPercent(int value)
{
    value = qBound(1, value, 200);
    if (m_nighttimeGammaPercent == value)
        return;
    m_nighttimeGammaPercent = value;
    setChanged();
}

void HyprsunsetInfo::reload()
{
    load();
}

void HyprsunsetInfo::load()
{
    m_available = !QStandardPaths::findExecutable(QStringLiteral("hyprsunset")).isEmpty();
    m_configAvailable = QFileInfo::exists(m_configPath) && QFileInfo(m_configPath).isFile();

    m_maxGamma = 150;
    m_daytimeHour = 7;
    m_daytimeMinute = 0;
    m_daytimeIdentity = true;
    m_nighttimeHour = 19;
    m_nighttimeMinute = 0;
    m_nighttimeTemperature = 5800;
    m_nighttimeGammaPercent = 80;

    QFile file(m_configPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        int profileIndex = -1;
        int nextProfileIndex = 0;
        const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char(10));
        for (const QString &line : lines)
        {
            const QString trimmed = line.trimmed();
            if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char(35)))
                continue;

            if (trimmed.startsWith(QStringLiteral("profile")) && trimmed.contains(QLatin1Char(123)))
            {
                profileIndex = nextProfileIndex++;
                continue;
            }

            if (profileIndex >= 0 && trimmed == QStringLiteral("}"))
            {
                profileIndex = -1;
                continue;
            }

            if (profileIndex < 0)
            {
                bool ok = false;
                const int maxGamma = assignmentValue(trimmed, QStringLiteral("max-gamma")).toInt(&ok);
                if (ok)
                    m_maxGamma = qBound(0, maxGamma, 200);
                continue;
            }

            const QString timeValue = assignmentValue(trimmed, QStringLiteral("time"));
            const QString identityValue = assignmentValue(trimmed, QStringLiteral("identity"));
            const QString temperatureValue = assignmentValue(trimmed, QStringLiteral("temperature"));
            const QString gammaValue = assignmentValue(trimmed, QStringLiteral("gamma"));

            if (!timeValue.isEmpty())
            {
                int hour = 0;
                int minute = 0;
                if (parseTime(timeValue, &hour, &minute))
                {
                    if (profileIndex == 0)
                    {
                        m_daytimeHour = hour;
                        m_daytimeMinute = minute;
                    }
                    else if (profileIndex == 1)
                    {
                        m_nighttimeHour = hour;
                        m_nighttimeMinute = minute;
                    }
                }
            }

            if (profileIndex == 0 && !identityValue.isEmpty())
                m_daytimeIdentity = parseBool(identityValue);

            if (profileIndex == 1)
            {
                bool temperatureOk = false;
                const int temperature = temperatureValue.toInt(&temperatureOk);
                if (temperatureOk)
                    m_nighttimeTemperature = qBound(1000, temperature, 10000);

                bool gammaOk = false;
                const double gamma = gammaValue.toDouble(&gammaOk);
                if (gammaOk)
                    m_nighttimeGammaPercent = qBound(1, qRound(gamma * 100.0), 200);
            }
        }
    }

    setChanged();
}

bool HyprsunsetInfo::save()
{
    const QFileInfo fileInfo(m_configPath);
    if (!QDir().mkpath(fileInfo.absolutePath()))
        return false;

    QSaveFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out << "max-gamma = " << m_maxGamma << "\n\n";
    out << "profile {\n";
    out << "    time = " << QStringLiteral("%1:%2").arg(m_daytimeHour, 2, 10, QLatin1Char(48)).arg(m_daytimeMinute, 2, 10, QLatin1Char(48)) << "\n";
    out << "    identity = " << (m_daytimeIdentity ? "true" : "false") << "\n";
    out << "}\n\n";
    out << "profile {\n";
    out << "    time = " << QStringLiteral("%1:%2").arg(m_nighttimeHour, 2, 10, QLatin1Char(48)).arg(m_nighttimeMinute, 2, 10, QLatin1Char(48)) << "\n";
    out << "    temperature = " << m_nighttimeTemperature << "\n";
    out << "    gamma = " << QString::number(m_nighttimeGammaPercent / 100.0, char(102), 2) << "\n";
    out << "}\n";

    if (!file.commit())
        return false;

    m_configAvailable = true;
    Q_EMIT settingsChanged();
    return true;
}
