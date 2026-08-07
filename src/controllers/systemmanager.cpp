#include "systemmanager.h"

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
#include <algorithm>
#include <QUrl>
#include <pwd.h>
#include <grp.h>

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
bool isSudoMember(const QString &username) { const QByteArray name = username.toUtf8(); const struct passwd *user = ::getpwnam(name.constData()); const struct group *sudo = ::getgrnam("sudo"); if (!user || !sudo) return false; if (user->pw_gid == sudo->gr_gid) return true; for (char **member = sudo->gr_mem; member && *member; ++member) if (username == QString::fromLocal8Bit(*member)) return true; return false; }
}

SystemManager::SystemManager(QObject *parent)
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
    reloadUsers();

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

QString SystemManager::timezone() const { return m_timezone; }
QStringList SystemManager::timezones() const { return m_timezones; }
QString SystemManager::locale() const { return m_locale; }
QStringList SystemManager::locales() const { return m_locales; }
QString SystemManager::hostName() const { return m_hostName; }
QVariantList SystemManager::users() const { return m_users; }
bool SystemManager::automaticLocation() const { return m_automaticLocation; }
bool SystemManager::busy() const { return m_busy; }
QString SystemManager::errorMessage() const { return m_errorMessage; }

void SystemManager::reloadUsers() {
    QVariantList users;
    ::setpwent();
    while (const struct passwd *entry = ::getpwent()) {
        if (entry->pw_uid < 1000 || entry->pw_uid == 65534) continue;
        QVariantMap user;
        user.insert(QStringLiteral("username"), QString::fromLocal8Bit(entry->pw_name));
        user.insert(QStringLiteral("name"), QString::fromLocal8Bit(entry->pw_gecos).section(QLatin1Char(','), 0, 0));
        user.insert(QStringLiteral("uid"), static_cast<quint32>(entry->pw_uid));
        const QString home = QString::fromLocal8Bit(entry->pw_dir);
        user.insert(QStringLiteral("home"), home);
        const QString facePath = home + QStringLiteral("/.face");
        if (QFileInfo::exists(facePath) && QFileInfo(facePath).isFile() && QFileInfo(facePath).isReadable()) user.insert(QStringLiteral("facePath"), QUrl::fromLocalFile(facePath).toString());
        users.append(user);
    }
    ::endpwent();
    int sudoUsers = 0;
    for (int i = 0; i < users.size(); ++i) { QVariantMap entry = users.at(i).toMap(); const bool administrator = isSudoMember(entry.value(QStringLiteral("username")).toString()); entry.insert(QStringLiteral("administrator"), administrator); if (administrator) ++sudoUsers; users[i] = entry; }
    for (int i = 0; i < users.size(); ++i) { QVariantMap entry = users.at(i).toMap(); const bool administrator = entry.value(QStringLiteral("administrator")).toBool(); entry.insert(QStringLiteral("canDelete"), users.size() > 1 && !(administrator && sudoUsers == 1)); users[i] = entry; }
    std::sort(users.begin(), users.end(), [](const QVariant &left, const QVariant &right) { return left.toMap().value(QStringLiteral("username")).toString().localeAwareCompare(right.toMap().value(QStringLiteral("username")).toString()) < 0; });
    m_users = users;
    Q_EMIT usersChanged();
}

void SystemManager::addUser(const QString &username, const QString &fullName, const QString &password, bool administrator, bool usePasswordQuality, const QString &avatarPath) {
    const QString value = username.trimmed();
    if (!QRegularExpression(QStringLiteral("^[a-z_][a-z0-9_-]{0,31}\\z")).match(value).hasMatch()) { setErrorMessage(QStringLiteral("Enter a valid username.")); return; }
    if (fullName.size() > 128) { setErrorMessage(QStringLiteral("The full name is too long.")); return; }
    if (administrator && password.isEmpty()) { setErrorMessage(QStringLiteral("Administrator accounts require a password.")); return; }
    const QString normalizedAvatar = avatarPath.startsWith(QStringLiteral("file://")) ? QUrl(avatarPath).toLocalFile() : avatarPath.trimmed();
    if (!normalizedAvatar.isEmpty() && (!QFileInfo(normalizedAvatar).isAbsolute() || !QFileInfo(normalizedAvatar).isFile() || !QFileInfo(normalizedAvatar).isReadable())) { setErrorMessage(QStringLiteral("Select a readable avatar image.")); return; }
    execute({{QStringLiteral("operation"), QStringLiteral("addUser")}, {QStringLiteral("username"), value}, {QStringLiteral("fullName"), fullName.trimmed()}, {QStringLiteral("password"), password}, {QStringLiteral("administrator"), administrator}, {QStringLiteral("usePasswordQuality"), usePasswordQuality}, {QStringLiteral("avatarPath"), normalizedAvatar}}, QStringLiteral("User added."));
}

void SystemManager::updateUser(const QString &username, const QString &password, const QString &avatarPath) {
    const QString value = username.trimmed();
    const QString normalizedAvatar = avatarPath.startsWith(QStringLiteral("file://")) ? QUrl(avatarPath).toLocalFile() : avatarPath.trimmed();
    if (!QRegularExpression(QStringLiteral("^[a-z_][a-z0-9_-]{0,31}\\z")).match(value).hasMatch()) { setErrorMessage(QStringLiteral("Enter a valid username.")); return; }
    if (password.contains(QChar::LineFeed) || password.contains(QChar::CarriageReturn)) { setErrorMessage(QStringLiteral("Enter a valid password.")); return; }
    if (password.isEmpty() && normalizedAvatar.isEmpty()) { setErrorMessage(QStringLiteral("Choose a new avatar or enter a new password.")); return; }
    if (!normalizedAvatar.isEmpty() && (!QFileInfo(normalizedAvatar).isAbsolute() || !QFileInfo(normalizedAvatar).isFile() || !QFileInfo(normalizedAvatar).isReadable())) { setErrorMessage(QStringLiteral("Select a readable avatar image.")); return; }
    execute({{QStringLiteral("operation"), QStringLiteral("updateUser")}, {QStringLiteral("username"), value}, {QStringLiteral("password"), password}, {QStringLiteral("avatarPath"), normalizedAvatar}}, QStringLiteral("User updated."));
}

void SystemManager::deleteUser(const QString &username) {
    const QString value = username.trimmed();
    for (const QVariant &user : m_users) if (user.toMap().value(QStringLiteral("username")).toString() == value && !user.toMap().value(QStringLiteral("canDelete")).toBool()) { setErrorMessage(QStringLiteral("This account cannot be deleted.")); return; }
    execute({{QStringLiteral("operation"), QStringLiteral("deleteUser")}, {QStringLiteral("username"), value}}, QStringLiteral("User deleted."));
}

void SystemManager::setAutomaticLocation(bool enabled)
{
    if (m_automaticLocation == enabled)
        return;
    m_automaticLocation = enabled;
    Q_EMIT automaticLocationChanged();
    if (enabled)
        lookupTimezoneByLocation();
}

void SystemManager::setTimezone(const QString &timezone)
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

void SystemManager::setLocale(const QString &locale) {
    if (!m_locales.contains(locale)) { setErrorMessage(QStringLiteral("The selected locale is not available.")); return; }
    execute({{QStringLiteral("operation"), QStringLiteral("locale")}, {QStringLiteral("locale"), locale}}, QStringLiteral("Locale updated."));
    m_locale = locale; Q_EMIT localeChanged();
}

void SystemManager::setHostName(const QString &hostName) {
    const QString value = hostName.trimmed();
    if (!QRegularExpression(QStringLiteral("^[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?(?:\\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*\\z")).match(value).hasMatch()) { setErrorMessage(QStringLiteral("Enter a valid hostname.")); return; }
    execute({{QStringLiteral("operation"), QStringLiteral("hostname")}, {QStringLiteral("hostname"), value}}, QStringLiteral("Hostname updated."));
    m_hostName = value; Q_EMIT hostNameChanged();
}

void SystemManager::setDateTime(const QString &isoDateTime)
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

void SystemManager::lookupTimezoneByLocation()
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

void SystemManager::execute(const QVariantMap &arguments, const QString &successMessage)
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

void SystemManager::setBusy(bool value)
{
    if (m_busy == value)
        return;
    m_busy = value;
    Q_EMIT busyChanged();
}

void SystemManager::setErrorMessage(const QString &value)
{
    if (m_errorMessage == value)
        return;
    m_errorMessage = value;
    Q_EMIT errorMessageChanged();
}
