#include "datetimemanager.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QSysInfo>
#include <QUrl>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>

namespace
{
constexpr auto timezoneFile = "/etc/timezone";
constexpr auto timezoneDirectory = "/usr/share/zoneinfo";
constexpr auto localeFile = "/etc/default/locale";
constexpr auto hostnameFile = "/etc/hostname";
constexpr auto helperId = "org.maui.settings.datetime";
constexpr auto actionId = "org.maui.settings.datetime.set";
}

DateTimeManager::DateTimeManager(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
    QFile file(QString::fromLatin1(timezoneFile));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        m_timezone = QString::fromUtf8(file.readAll()).trimmed();

    if (m_timezone.isEmpty())
    {
        const QFileInfo localtime(QStringLiteral("/etc/localtime"));
        const QString marker = QStringLiteral("/zoneinfo/");
        const QString path = localtime.symLinkTarget();
        const int index = path.indexOf(marker);
        if (index >= 0)
            m_timezone = path.mid(index + marker.size());
    }

    QFile localeSettings(QString::fromLatin1(localeFile));
    if (localeSettings.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString contents = QString::fromUtf8(localeSettings.readAll());
        const auto match = QRegularExpression(QStringLiteral("^\\s*LANG\\s*=\\s*[\"\\x27]?([^\"\\x27\\n]+)")).match(contents);
        if (match.hasMatch()) m_locale = match.captured(1).trimmed();
    }
    QFile hostnameSettings(QString::fromLatin1(hostnameFile));
    if (hostnameSettings.open(QIODevice::ReadOnly | QIODevice::Text)) m_hostName = QString::fromUtf8(hostnameSettings.readAll()).trimmed();
    if (m_hostName.isEmpty()) m_hostName = QSysInfo::machineHostName();

    QProcess localeProcess;
    localeProcess.start(QStringLiteral("locale"), {QStringLiteral("-a")});
    if (localeProcess.waitForFinished(5000) && localeProcess.exitCode() == 0) {
        const auto available = QString::fromUtf8(localeProcess.readAllStandardOutput()).split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
        for (const auto &value : available) if (!value.trimmed().isEmpty() && !m_locales.contains(value.trimmed())) m_locales.append(value.trimmed());
    }
    if (m_locale.isEmpty()) m_locale = qEnvironmentVariable("LANG");
    if (m_locale.isEmpty()) m_locale = QStringLiteral("C.UTF-8");
    if (!m_locales.contains(m_locale)) m_locales.append(m_locale);
    m_locales.sort(Qt::CaseInsensitive);

    QDirIterator iterator(QString::fromLatin1(timezoneDirectory), QDir::Files,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        const QString path = iterator.next();
        const QString relative = QDir(QString::fromLatin1(timezoneDirectory)).relativeFilePath(path);
        if (!relative.startsWith(QStringLiteral("posix/"))
            && !relative.startsWith(QStringLiteral("right/"))
            && !relative.contains(QStringLiteral("/posix/"))
            && !relative.contains(QStringLiteral("/right/")))
            m_timezones.append(relative);
    }
    m_timezones.removeDuplicates();
    m_timezones.sort(Qt::CaseInsensitive);
}

QString DateTimeManager::timezone() const { return m_timezone; }
QStringList DateTimeManager::timezones() const { return m_timezones; }
QString DateTimeManager::locale() const { return m_locale; }
QStringList DateTimeManager::locales() const { return m_locales; }
QString DateTimeManager::hostName() const { return m_hostName; }
bool DateTimeManager::automaticLocation() const { return m_automaticLocation; }
bool DateTimeManager::busy() const { return m_busy; }
QString DateTimeManager::errorMessage() const { return m_errorMessage; }

void DateTimeManager::setAutomaticLocation(bool enabled)
{
    if (m_automaticLocation == enabled)
        return;
    m_automaticLocation = enabled;
    Q_EMIT automaticLocationChanged();
    if (enabled)
        lookupTimezoneByLocation();
}

void DateTimeManager::setTimezone(const QString &timezone)
{
    if (!m_timezones.contains(timezone))
    {
        setErrorMessage(QStringLiteral("The selected timezone is not valid."));
        return;
    }
    execute({{QStringLiteral("operation"), QStringLiteral("timezone")},
             {QStringLiteral("timezone"), timezone}},
            QStringLiteral("Timezone updated."));
    m_timezone = timezone;
    Q_EMIT timezoneChanged();
}

void DateTimeManager::setLocale(const QString &locale) {
    if (!m_locales.contains(locale)) { setErrorMessage(QStringLiteral("The selected locale is not available.")); return; }
    execute({{QStringLiteral("operation"), QStringLiteral("locale")}, {QStringLiteral("locale"), locale}}, QStringLiteral("Locale updated."));
    m_locale = locale; Q_EMIT localeChanged();
}

void DateTimeManager::setHostName(const QString &hostName) {
    const QString value = hostName.trimmed();
    if (!QRegularExpression(QStringLiteral("^[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?(?:\\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*\\z")).match(value).hasMatch()) { setErrorMessage(QStringLiteral("Enter a valid hostname.")); return; }
    execute({{QStringLiteral("operation"), QStringLiteral("hostname")}, {QStringLiteral("hostname"), value}}, QStringLiteral("Hostname updated."));
    m_hostName = value; Q_EMIT hostNameChanged();
}

void DateTimeManager::setDateTime(const QString &isoDateTime)
{
    const QDateTime dateTime = QDateTime::fromString(isoDateTime, Qt::ISODate);
    if (!dateTime.isValid())
    {
        setErrorMessage(QStringLiteral("Enter a valid date and time."));
        return;
    }
    execute({{QStringLiteral("operation"), QStringLiteral("clock")},
             {QStringLiteral("epochSeconds"), dateTime.toSecsSinceEpoch()}},
            QStringLiteral("System clock updated."));
}

void DateTimeManager::lookupTimezoneByLocation()
{
    if (m_busy)
        return;
    setErrorMessage(QString());
    QNetworkReply *reply = m_network->get(QNetworkRequest(QUrl(QStringLiteral("https://ipapi.co/timezone/"))));
    connect(reply, &QNetworkReply::finished, this, [this, reply]
    {
        const QString zone = QString::fromUtf8(reply->readAll()).trimmed();
        const bool ok = reply->error() == QNetworkReply::NoError && m_timezones.contains(zone);
        reply->deleteLater();
        if (!ok)
        {
            setErrorMessage(QStringLiteral("Could not determine the timezone from your network location."));
            return;
        }
        setTimezone(zone);
    });
}

void DateTimeManager::execute(const QVariantMap &arguments, const QString &successMessage)
{
    if (m_busy)
        return;
    setBusy(true);
    setErrorMessage(QString());
    KAuth::Action action(QString::fromLatin1(actionId));
    action.setHelperId(QString::fromLatin1(helperId));
    action.setArguments(arguments);
    if (QWindow *window = QGuiApplication::focusWindow())
        action.setParentWindow(window);
    KAuth::ExecuteJob *job = action.execute();
    connect(job, &KJob::result, this, [this, job, successMessage]
    {
        setBusy(false);
        if (job->error() != 0)
        {
            setErrorMessage(job->errorText().isEmpty()
                ? QStringLiteral("Authorization or the privileged clock operation failed.")
                : job->errorText());
            return;
        }
        Q_EMIT operationSucceeded(successMessage);
    });
    job->start();
}

void DateTimeManager::setBusy(bool value)
{
    if (m_busy == value)
        return;
    m_busy = value;
    Q_EMIT busyChanged();
}

void DateTimeManager::setErrorMessage(const QString &value)
{
    if (m_errorMessage == value)
        return;
    m_errorMessage = value;
    Q_EMIT errorMessageChanged();
}
