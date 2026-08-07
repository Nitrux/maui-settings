#include "datetimehelper.h"
#include "systemfilepersistence.h"
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStringList>
#include <KAuth/HelperSupport>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#ifdef MAUI_SETTINGS_HAS_PWQUALITY
extern "C" {
#include "pwquality.h"
}
#endif
namespace {
constexpr auto localtimePath = "/etc/localtime"; constexpr auto passwordQualityConfig = "/etc/security/pwquality.conf"; constexpr auto timezonePath = "/etc/timezone"; constexpr auto localePath = "/etc/default/locale"; constexpr auto hostnamePath = "/etc/hostname"; constexpr auto zoneinfoPath = "/usr/share/zoneinfo/";
KAuth::ActionReply error(const QString &message, int code) { auto r=KAuth::ActionReply::HelperErrorReply(); r.setError(code); r.setErrorDescription(message); return r; }
bool valid(const QString &z) { if(z.isEmpty()||z.startsWith('/')||z.contains("..")) return false; QFileInfo i(QString::fromLatin1(zoneinfoPath)+z); return i.exists()&&i.isFile()&&i.isReadable(); }
bool updateTimezone(const QString &z, QString *e) { const QString target=QString::fromLatin1(zoneinfoPath)+z; QFile::remove(localtimePath); if(!QFile::link(target,localtimePath)){*e="Could not update /etc/localtime.";return false;} QSaveFile f(timezonePath); if(!f.open(QIODevice::WriteOnly|QIODevice::Text)||f.write(z.toUtf8()+QByteArray("\n"))<0||!f.commit()){*e="Could not update /etc/timezone.";return false;} QString pe; if(!SystemFilePersistence::persistSymlink(target,localtimePath,&pe)||!SystemFilePersistence::persist(timezonePath,&pe)){*e=QStringLiteral("Timezone changed for this session but could not be persisted: %1").arg(pe);return false;} return true; }
bool clock(qint64 s, QString *e) { timespec t{s,0}; if(::clock_settime(CLOCK_REALTIME,&t)!=0){*e=QStringLiteral("Could not set the system clock: %1").arg(QString::fromLocal8Bit(std::strerror(errno)));return false;} QProcess p; p.start("/sbin/hwclock",{"--systohc"}); if(!p.waitForFinished(10000)||p.exitCode()!=0){*e="System clock changed, but the hardware clock could not be synchronized.";return false;} return true; }
bool persistAccountFiles(QString *e) {
    const QStringList files = {QStringLiteral("/etc/passwd"), QStringLiteral("/etc/shadow"), QStringLiteral("/etc/group"), QStringLiteral("/etc/gshadow")};
    for (const QString &path : files) {
        if (!QFileInfo::exists(path)) continue;
        QString persistenceError;
        if (!SystemFilePersistence::persist(path, &persistenceError)) { *e = persistenceError; return false; }
    }
    return true;
}

bool validatePassword(const QString &username, const QString &password, QString *e) {
#ifdef MAUI_SETTINGS_HAS_PWQUALITY
    pwquality_settings_t *settings = pwquality_default_settings();
    if (!settings) { *e = "Could not initialize password quality checks."; return false; }
    void *auxError = nullptr;
    int result = pwquality_read_config(settings, passwordQualityConfig, &auxError);
    if (result != 0) { *e = QString::fromUtf8(pwquality_strerror(nullptr, 0, result, auxError)); pwquality_free_settings(settings); return false; }
    const QByteArray usernameBytes = username.toUtf8();
    const QByteArray passwordBytes = password.toUtf8();
    result = pwquality_check(settings, passwordBytes.constData(), nullptr, usernameBytes.constData(), &auxError);
    if (result < 0) { *e = QString::fromUtf8(pwquality_strerror(nullptr, 0, result, auxError)); pwquality_free_settings(settings); return false; }
    pwquality_free_settings(settings);
#else
    Q_UNUSED(username)
    Q_UNUSED(password)
    Q_UNUSED(e)
#endif
    return true;
}

bool addUser(const QString &username, const QString &fullName, const QString &password, bool administrator, bool usePasswordQuality, const QString &avatarPath, QString *e) {
    if (!QRegularExpression(QStringLiteral("^[a-z_][a-z0-9_-]{0,31}\\z")).match(username).hasMatch()) { *e = "Invalid username."; return false; }
    if (administrator && password.isEmpty()) { *e = "Administrator accounts require a password."; return false; }
    if (fullName.size() > 128 || password.contains(QChar::LineFeed) || password.contains(QChar::CarriageReturn)) { *e = "Invalid user data."; return false; }
    if (usePasswordQuality && !validatePassword(username, password, e)) return false;
    QProcess userAdd;
    userAdd.setProgram(QStringLiteral("/usr/sbin/useradd"));
    QStringList userArguments = {QStringLiteral("--create-home"), QStringLiteral("--comment"), fullName};
    if (administrator) userArguments << QStringLiteral("--groups") << QStringLiteral("sudo");
    userArguments << username;
    userAdd.setArguments(userArguments);
    userAdd.start();
    if (!userAdd.waitForFinished(30000) || userAdd.exitStatus() != QProcess::NormalExit || userAdd.exitCode() != 0) { *e = QString::fromLocal8Bit(userAdd.readAllStandardError()).trimmed(); if (e->isEmpty()) *e = "Could not create the user account."; return false; }
    if (!password.isEmpty()) {
        QProcess passwordProcess;
        passwordProcess.setProgram(QStringLiteral("/usr/sbin/chpasswd"));
        passwordProcess.start();
        if (!passwordProcess.waitForStarted(5000)) { *e = "User created, but the password process could not start."; return false; }
        if (passwordProcess.write(username.toUtf8() + QByteArray(":") + password.toUtf8() + QByteArray("\n")) < 0) { *e = "User created, but the password could not be set."; return false; }
        passwordProcess.closeWriteChannel();
        if (!passwordProcess.waitForFinished(30000) || passwordProcess.exitCode() != 0) { *e = "User created, but the password could not be set."; return false; }
    }
    if (!avatarPath.isEmpty()) {
        const struct passwd *account = ::getpwnam(username.toUtf8().constData());
        if (!account) { *e = "User created, but its account entry could not be read."; return false; }
        const QString destination = QString::fromLocal8Bit(account->pw_dir) + QStringLiteral("/.face");
        QFile::remove(destination);
        if (!QFile::copy(avatarPath, destination)) { *e = "User created, but the avatar could not be installed."; return false; }
        ::chown(destination.toUtf8().constData(), account->pw_uid, account->pw_gid);
        if (!SystemFilePersistence::persist(destination, e)) return false;
    }
    if (!persistAccountFiles(e)) { *e = QStringLiteral("User created for this session but could not be persisted: %1").arg(*e); return false; }
    return true;
}

bool sudoMember(const struct passwd *account) { const struct group *sudo = ::getgrnam("sudo"); if (!account || !sudo) return false; if (account->pw_gid == sudo->gr_gid) return true; for (char **member = sudo->gr_mem; member && *member; ++member) if (std::strcmp(account->pw_name, *member) == 0) return true; return false; }
bool accountCanBeDeleted(const QString &username, QString *e) { const QByteArray name = username.toUtf8(); const struct passwd *target = ::getpwnam(name.constData()); if (!target || target->pw_uid < 1000 || target->pw_uid == 65534) { *e = "This account cannot be deleted."; return false; } int regular = 0, sudo = 0; bool targetSudo = false; ::setpwent(); while (const struct passwd *account = ::getpwent()) { if (account->pw_uid < 1000 || account->pw_uid == 65534) continue; ++regular; if (sudoMember(account)) { ++sudo; if (username == QString::fromLocal8Bit(account->pw_name)) targetSudo = true; } } ::endpwent(); if (regular <= 1 || (targetSudo && sudo <= 1)) { *e = "This account cannot be deleted."; return false; } return true; }
bool updateUserAccount(const QString &username, const QString &password, const QString &avatarPath, QString *e) { const struct passwd *account = ::getpwnam(username.toUtf8().constData()); if (!account) { *e = "The user account could not be found."; return false; } if (!password.isEmpty() && !validatePassword(username, password, e)) return false; if (!password.isEmpty()) { QProcess process; process.setProgram(QStringLiteral("/usr/sbin/chpasswd")); process.start(); if (!process.waitForStarted(5000) || process.write(username.toUtf8() + QByteArray(":") + password.toUtf8() + QByteArray("\n")) < 0) { *e = "The password could not be set."; return false; } process.closeWriteChannel(); if (!process.waitForFinished(30000) || process.exitCode() != 0) { *e = "The password could not be set."; return false; } } if (!avatarPath.isEmpty()) { const QString destination = QString::fromLocal8Bit(account->pw_dir) + QStringLiteral("/.face"); QFile::remove(destination); if (!QFile::copy(avatarPath, destination)) { *e = "The avatar could not be installed."; return false; } ::chown(destination.toUtf8().constData(), account->pw_uid, account->pw_gid); if (!SystemFilePersistence::persist(destination, e)) return false; } return persistAccountFiles(e); }
bool deleteUserAccount(const QString &username, QString *e) { if (!accountCanBeDeleted(username, e)) return false; QProcess process; process.setProgram(QStringLiteral("/usr/sbin/userdel")); process.setArguments({QStringLiteral("--remove"), username}); process.start(); if (!process.waitForFinished(30000) || process.exitCode() != 0) { *e = QString::fromLocal8Bit(process.readAllStandardError()).trimmed(); if (e->isEmpty()) *e = "Could not delete the user account."; return false; } return persistAccountFiles(e); }
bool updateHostname(const QString &hostname, QString *e) {
    if (!QRegularExpression(QStringLiteral("^[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?(?:\\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*\\z")).match(hostname).hasMatch()) { *e = "Invalid hostname."; return false; }
    const QByteArray encoded = hostname.toUtf8();
    if (::sethostname(encoded.constData(), static_cast<size_t>(encoded.size())) != 0) { *e = QStringLiteral("Could not update the running hostname: %1").arg(QString::fromLocal8Bit(std::strerror(errno))); return false; }
    QSaveFile file(QString::fromLatin1(hostnamePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text) || file.write(encoded + QByteArray("\n")) < 0 || !file.commit()) { *e = "Could not update /etc/hostname."; return false; }
    QString persistenceError;
    if (!SystemFilePersistence::persist(QString::fromLatin1(hostnamePath), &persistenceError)) { *e = QStringLiteral("Hostname changed for this session but could not be persisted: %1").arg(persistenceError); return false; }
    return true;
}

bool updateLocale(const QString &locale, QString *e) {
    if (!QRegularExpression(QStringLiteral("^[A-Za-z]{1,3}([_.@-][A-Za-z0-9]+)*\\z")).match(locale).hasMatch()) { *e = "Invalid locale."; return false; }
    QSaveFile file(QString::fromLatin1(localePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text) || file.write(QByteArray("LANG=") + locale.toUtf8() + QByteArray("\n")) < 0 || !file.commit()) { *e = "Could not update /etc/default/locale."; return false; }
    QString persistenceError;
    if (!SystemFilePersistence::persist(QString::fromLatin1(localePath), &persistenceError)) { *e = QStringLiteral("Locale changed for this session but could not be persisted: %1").arg(persistenceError); return false; }
    return true;
}
}
KAuth::ActionReply DateTimeHelper::set(const QVariantMap &a) { QString e; const QString op=a.value("operation").toString(); if(op=="timezone"){const QString z=a.value("timezone").toString().trimmed();if(!valid(z))return error("Invalid IANA timezone.",1001);if(!updateTimezone(z,&e))return error(e,1002);}else if(op=="clock"){const qint64 s=a.value("epochSeconds").toLongLong();if(s<0||!clock(s,&e))return error(e.isEmpty()?"Invalid clock value.":e,1003);}else if(op=="hostname"){if(!updateHostname(a.value("hostname").toString().trimmed(),&e))return error(e,1005);}else if(op=="addUser"){if(!addUser(a.value("username").toString().trimmed(),a.value("fullName").toString().trimmed(),a.value("password").toString(),a.value("administrator").toBool(),a.value("usePasswordQuality").toBool(),a.value("avatarPath").toString(),&e))return error(e,1006);}else if(op=="updateUser"){if(!updateUserAccount(a.value("username").toString().trimmed(),a.value("password").toString(),a.value("avatarPath").toString(),&e))return error(e,1007);}else if(op=="deleteUser"){if(!deleteUserAccount(a.value("username").toString().trimmed(),&e))return error(e,1008);}else if(op=="locale"){if(!updateLocale(a.value("locale").toString().trimmed(),&e))return error(e,1004);}else return error("Unknown System operation.",1000);return KAuth::ActionReply::SuccessReply();}
KAUTH_HELPER_MAIN("org.maui.settings.datetime", DateTimeHelper)
