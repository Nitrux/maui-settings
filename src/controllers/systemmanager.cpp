#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusObjectPath>
#include "systemmanager.h"

extern "C" {
#include <pwquality.h>
}

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
#include <QProcessEnvironment>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSysInfo>
#include <algorithm>
#include <QUrl>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

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
QString credentialTool(const QString &operation)
{
    if (operation == "setup" || operation == "change" || operation == "delete" || operation == "status")
        return QStringLiteral("/usr/bin/pinutil");
    if (operation == "enroll")
        return QStringLiteral("/usr/bin/fprintd-enroll");
    if (operation == "removePrints")
        return QStringLiteral("/usr/bin/fprintd-delete");
    return {};
}
bool isSudoMember(const QString &username) { const QByteArray name = username.toUtf8(); const struct passwd *user = ::getpwnam(name.constData()); const struct group *sudo = ::getgrnam("sudo"); if (!user || !sudo) return false; if (user->pw_gid == sudo->gr_gid) return true; for (char **member = sudo->gr_mem; member && *member; ++member) if (username == QString::fromLocal8Bit(*member)) return true; return false; }
}

SystemManager::SystemManager(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
}

void SystemManager::initialize()
{
    if (m_initialized)
        return;

    m_initialized = true;
    manageCredential("status", {}, {});
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

    Q_EMIT timezoneChanged();
    Q_EMIT timezonesChanged();
    Q_EMIT localeChanged();
    Q_EMIT localesChanged();
    Q_EMIT hostNameChanged();
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

int SystemManager::passwordScore(const QString &password, const QString &username) const
{
    auto *settings = pwquality_default_settings();
    if (!settings)
        return -1;
    int score = pwquality_read_config(settings, "/etc/security/pwquality.conf", nullptr);
    if (score == 0)
        score = pwquality_check(settings, password.toUtf8().constData(), nullptr,
                                username.toUtf8().constData(), nullptr);
    pwquality_free_settings(settings);
    return score;
}

QString SystemManager::credentialUser() const
{
    const passwd *account = ::getpwuid(::getuid());
    return account ? QString::fromLocal8Bit(account->pw_name) : QString();
}

bool SystemManager::credentialToolAvailable(const QString &operation) const
{
    return QFileInfo(credentialTool(operation)).isExecutable();
}

static bool fingerprintReaderAvailable()
{
    const auto request = QDBusMessage::createMethodCall(
        QStringLiteral("net.reactivated.Fprint"), QStringLiteral("/net/reactivated/Fprint/Manager"),
        QStringLiteral("net.reactivated.Fprint.Manager"), QStringLiteral("GetDefaultDevice"));
    const QDBusReply<QDBusObjectPath> device = QDBusConnection::systemBus().call(request, QDBus::Block, 2000);
    return device.isValid() && !device.value().path().isEmpty();
}

void SystemManager::refreshFingerprints()
{
    const QString user = credentialUser();
    const bool reader = fingerprintReaderAvailable();
    Q_EMIT fingerprintReaderChanged(reader);
    if (!reader || user.isEmpty()) {
        Q_EMIT fingerprintsListed({}, false);
        return;
    }
    auto *process = new QProcess(this);
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    process->setProcessEnvironment(environment);
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            Q_EMIT fingerprintsListed({}, false);
            process->deleteLater();
        }
    });
    connect(process, &QProcess::finished, this, [this, process](int code, QProcess::ExitStatus status) {
        const QString output = QString::fromUtf8(process->readAllStandardOutput());
        static const QRegularExpression finger(QStringLiteral(R"(^[ \t]*(?:-[ \t]*)?#\d+:[ \t]*([a-z-]+)[ \t]*$)"),
                                                QRegularExpression::MultilineOption);
        QStringList fingers;
        auto matches = finger.globalMatch(output);
        while (matches.hasNext())
            fingers.append(matches.next().captured(1).replace('-', ' '));
        Q_EMIT fingerprintsListed(fingers, status == QProcess::NormalExit && code == 0
                                  && !output.contains(QStringLiteral("No devices available")));
        process->deleteLater();
    });
    process->start(QStringLiteral("/usr/bin/fprintd-list"), {user});
}

void SystemManager::manageCredential(const QString &operation, const QString &currentPin,
                                     const QString &newPin)
{
    const QString user = credentialUser();
    if (busy() || user.isEmpty() || !credentialToolAvailable(operation))
        return;
    const QString program = credentialTool(operation);
    QStringList args;
    QByteArray input;
    if (program.endsWith("/pinutil")) {
        static const QRegularExpression digits(QStringLiteral("^[0-9]+\\z"));
        if (operation != "setup" && operation != "status" && ::getuid() != 0) {
            if (!digits.match(currentPin).hasMatch())
                return;
            input += currentPin.toLatin1() + '\n';
        }
        if (operation != "delete" && operation != "status") {
            if (!digits.match(newPin).hasMatch())
                return;
            input += newPin.toLatin1() + '\n';
        }
        args << "--machine" << operation;
    } else {
        args << user;
    }
    auto *process = new QProcess(this);
    if (!program.endsWith("/pinutil"))
        process->setProcessChannelMode(QProcess::MergedChannels);
    setBusy(true);
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process, program] {
        if (program.endsWith("/pinutil"))
            return;
        const QString output = QString::fromLocal8Bit(process->readAllStandardOutput()).trimmed();
        if (!output.isEmpty())
            Q_EMIT credentialMessage(output);
    });
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            Q_EMIT credentialMessage(process->errorString());
            setBusy(false);
            process->deleteLater();
        }
    });
    connect(process, &QProcess::finished, this, [this, process, program, operation](int code, QProcess::ExitStatus status) {
        if (program.endsWith("/pinutil") && code == 0 && status == QProcess::NormalExit) {
            if (!process->arguments().contains("status")) {
                process->readAllStandardOutput();
                process->start(program, {"--machine", "status"});
                process->closeWriteChannel();
                return;
            }
            const auto result = QJsonDocument::fromJson(process->readAllStandardOutput()).object().value("Ok");
            const bool enabled = result.isObject();
            if (m_pinEnabled != enabled) {
                m_pinEnabled = enabled;
                Q_EMIT pinEnabledChanged();
            }
            if (!enabled && !result.isNull())
                code = 1;
            else if (operation != "status" && enabled != (operation != "delete"))
                code = 1;
        }
        if (operation != "status") {
            if (code != 0 || status != QProcess::NormalExit)
                Q_EMIT credentialMessage(tr("Credential operation failed."));
            else if (program.endsWith("/pinutil"))
                Q_EMIT pinOperationSucceeded();
        }
        setBusy(false);
        if (operation == "enroll" || operation == "removePrints") refreshFingerprints();
        process->deleteLater();
    });
    process->start(program, args);
    if (!input.isEmpty()) {
        process->write(input);
        input.fill(0);
    }
    process->closeWriteChannel();
}

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
        QString email;
        const auto fields = QString::fromLocal8Bit(entry->pw_gecos).split(QLatin1Char(','));
        for (const auto &field : fields.mid(4))
            if (field.startsWith(QStringLiteral("email="))) email = field.mid(6);
        user.insert(QStringLiteral("email"), email);
        QStringList groups;
        ::setgrent();
        while (const auto *group = ::getgrent()) {
            for (char **member = group->gr_mem; member && *member; ++member)
                if (user.value(QStringLiteral("username")).toString() == QString::fromLocal8Bit(*member))
                    groups.append(QString::fromLocal8Bit(group->gr_name));
        }
        ::endgrent();
        user.insert(QStringLiteral("groups"), groups.join(QStringLiteral(", ")));
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

void SystemManager::updateProfile(const QString &username, const QString &name, const QString &email,
                                  const QString &groups, bool groupsChanged, const QString &avatar)
{
    QVariantMap args{{"operation", "profile"}, {"username", username}, {"name", name.trimmed()},
                     {"email", email.trimmed()}, {"avatar", avatar.startsWith("file://") ? QUrl(avatar).toLocalFile() : avatar}};
    if (groupsChanged) args.insert("groups", groups);
    execute(args, QStringLiteral("User updated."));
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
