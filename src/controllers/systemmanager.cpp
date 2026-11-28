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
#include <QDBusPendingCallWatcher>
#include <QDBusVariant>
#include <QUuid>
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

namespace
{
constexpr auto fprintService = "net.reactivated.Fprint";
constexpr auto deviceInterface = "net.reactivated.Fprint.Device";

QDBusMessage deviceRequest(const QString &path, const QString &method, const QVariantList &args = {})
{
    auto request = QDBusMessage::createMethodCall(fprintService, path, deviceInterface, method);
    request.setInteractiveAuthorizationAllowed(method == "Claim" || method == "EnrollStart" || method == "DeleteEnrolledFinger");
    request.setArguments(args);
    return request;
}
}

void SystemManager::refreshFingerprints()
{
    const auto bus = QDBusConnection::systemBus();
    const auto request = QDBusMessage::createMethodCall(fprintService, QStringLiteral("/net/reactivated/Fprint/Manager"),
                                                        QStringLiteral("net.reactivated.Fprint.Manager"), QStringLiteral("GetDefaultDevice"));
    const QDBusReply<QDBusObjectPath> reader = bus.call(request, QDBus::Block, 2000);
    m_readerPath = reader.isValid() ? reader.value().path() : QString();
    Q_EMIT fingerprintReaderChanged(!m_readerPath.isEmpty());
    if (m_readerPath.isEmpty() || credentialUser().isEmpty()) {
        Q_EMIT fingerprintsListed({}, false);
        return;
    }
    const QDBusReply<QStringList> list = bus.call(deviceRequest(m_readerPath, "ListEnrolledFingers", {credentialUser()}), QDBus::Block, 2000);
    Q_EMIT fingerprintsListed(list.value(), list.isValid() || list.error().name() == "net.reactivated.Fprint.Error.NoEnrolledPrints");
}

SystemManager::~SystemManager()
{
    cancelFingerprintEnrollment();
}

void SystemManager::removeFingerprint(const QString &finger)
{
    const QString path = m_readerPath;
    const QString user = credentialUser();
    if (busy() || path.isEmpty() || user.isEmpty() || finger.isEmpty())
        return;

    const QString session = QUuid::createUuid().toString();
    const auto bus = QDBusConnection::connectToBus(QDBusConnection::SystemBus, session);
    auto *operation = new QObject(this);
    connect(operation, &QObject::destroyed, [session] { QDBusConnection::disconnectFromBus(session); });
    setBusy(true);
    const auto finish = [this, operation](const QString &error) {
        operation->deleteLater();
        refreshFingerprints();
        setBusy(false);
        Q_EMIT credentialMessage(error.isEmpty() ? tr("Fingerprint removed.") : error);
    };
    auto *claim = new QDBusPendingCallWatcher(bus.asyncCall(deviceRequest(path, "Claim", {user})), operation);
    connect(claim, &QDBusPendingCallWatcher::finished, this, [operation, bus, path, finger, finish](QDBusPendingCallWatcher *reply) {
        if (reply->isError()) {
            finish(reply->error().message());
            return;
        }
        auto *remove = new QDBusPendingCallWatcher(bus.asyncCall(deviceRequest(path, "DeleteEnrolledFinger", {finger})), operation);
        connect(remove, &QDBusPendingCallWatcher::finished, operation, [operation, bus, path, finish](QDBusPendingCallWatcher *result) {
            const QString error = result->error().message();
            auto *release = new QDBusPendingCallWatcher(bus.asyncCall(deviceRequest(path, "Release")), operation);
            connect(release, &QDBusPendingCallWatcher::finished, operation, [error, finish](QDBusPendingCallWatcher *released) {
                finish(error.isEmpty() ? released->error().message() : error);
            });
        });
    });
}

void SystemManager::enrollFingerprint(const QString &finger)
{
    if (busy() || m_readerPath.isEmpty()) {
        Q_EMIT enrollmentFinished(false, tr("The fingerprint reader is unavailable."));
        return;
    }

    // A separate bus connection ties the claim to this enrollment, including during authorization.
    const QString session = QUuid::createUuid().toString();
    auto bus = QDBusConnection::connectToBus(QDBusConnection::SystemBus, session);
    m_enrollmentConnection = session;
    m_enrollmentPath = m_readerPath;
    setBusy(true);
    if (!bus.connect(fprintService, m_enrollmentPath, deviceInterface, "EnrollStatus", this,
                     SLOT(handleEnrollStatus(QString,bool)))) {
        cancelFingerprintEnrollment();
        Q_EMIT enrollmentFinished(false, tr("Could not connect to the fingerprint reader."));
        return;
    }

    auto *claim = new QDBusPendingCallWatcher(bus.asyncCall(deviceRequest(m_enrollmentPath, "Claim", {credentialUser()})), this);
    connect(claim, &QDBusPendingCallWatcher::finished, this, [this, session, finger](QDBusPendingCallWatcher *reply) {
        reply->deleteLater();
        if (session != m_enrollmentConnection)
            return;
        if (reply->isError()) {
            const QString reason = reply->error().message();
            cancelFingerprintEnrollment();
            Q_EMIT enrollmentFinished(false, reason);
            return;
        }
        const QDBusConnection bus(session);
        const auto property = [&](const QString &name) {
            auto request = QDBusMessage::createMethodCall(fprintService, m_enrollmentPath, "org.freedesktop.DBus.Properties", "Get");
            request << QString::fromLatin1(deviceInterface) << name;
            const QDBusReply<QDBusVariant> value = bus.call(request, QDBus::Block, 2000);
            return value.value().variant();
        };
        const QString scanType = property("scan-type").toString();
        const int stages = property("num-enroll-stages").toInt();
        auto *start = new QDBusPendingCallWatcher(bus.asyncCall(deviceRequest(m_enrollmentPath, "EnrollStart", {finger})), this);
        connect(start, &QDBusPendingCallWatcher::finished, this, [this, session, scanType, stages](QDBusPendingCallWatcher *result) {
            result->deleteLater();
            if (session != m_enrollmentConnection)
                return;
            if (result->isError()) {
                const QString reason = result->error().message();
                cancelFingerprintEnrollment();
                Q_EMIT enrollmentFinished(false, reason);
                return;
            }
            Q_EMIT enrollmentReady(scanType, stages);
        });
    });
}

void SystemManager::cancelFingerprintEnrollment()
{
    if (m_enrollmentConnection.isEmpty())
        return;
    QDBusConnection bus(m_enrollmentConnection);
    bus.disconnect(fprintService, m_enrollmentPath, deviceInterface, "EnrollStatus", this, SLOT(handleEnrollStatus(QString,bool)));
    bus.send(deviceRequest(m_enrollmentPath, "EnrollStop"));
    bus.send(deviceRequest(m_enrollmentPath, "Release"));
    QDBusConnection::disconnectFromBus(m_enrollmentConnection);
    m_enrollmentConnection.clear();
    m_enrollmentPath.clear();
    setBusy(false);
}

void SystemManager::handleEnrollStatus(const QString &result, bool done)
{
    if (m_enrollmentConnection.isEmpty())
        return;
    if (done) {
        const bool success = result == "enroll-completed";
        QString reason;
        if (result == "enroll-data-full")
            reason = tr("The reader is full. Remove an enrolled fingerprint and try again.");
        else if (result == "enroll-disconnected")
            reason = tr("The reader was disconnected. Reconnect it and try again.");
        else if (result == "enroll-duplicate")
            reason = tr("This finger is already enrolled. Choose another finger.");
        else if (!success)
            reason = tr("The reader could not enroll this finger. Try again.");
        cancelFingerprintEnrollment();
        refreshFingerprints();
        Q_EMIT enrollmentFinished(success, reason);
    } else if (result == "enroll-stage-passed") {
        Q_EMIT enrollmentScanAccepted();
    } else if (result == "enroll-swipe-too-short") {
        Q_EMIT enrollmentFeedback(tr("Swipe the full length of your finger across the reader."));
    } else if (result == "enroll-finger-not-centered") {
        Q_EMIT enrollmentFeedback(tr("Center your finger on the reader and try again."));
    } else if (result == "enroll-remove-and-retry") {
        Q_EMIT enrollmentFeedback(tr("Lift your finger, then scan it again."));
    } else {
        Q_EMIT enrollmentFeedback(tr("Scan your finger again."));
    }
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
        if (operation == "removePrints") refreshFingerprints();
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
