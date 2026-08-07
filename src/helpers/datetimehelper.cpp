#include "datetimehelper.h"
#include "systemfilepersistence.h"
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <KAuth/HelperSupport>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <unistd.h>
namespace {
constexpr auto localtimePath = "/etc/localtime"; constexpr auto timezonePath = "/etc/timezone"; constexpr auto localePath = "/etc/default/locale"; constexpr auto hostnamePath = "/etc/hostname"; constexpr auto zoneinfoPath = "/usr/share/zoneinfo/";
KAuth::ActionReply error(const QString &message, int code) { auto r=KAuth::ActionReply::HelperErrorReply(); r.setError(code); r.setErrorDescription(message); return r; }
bool valid(const QString &z) { if(z.isEmpty()||z.startsWith('/')||z.contains("..")) return false; QFileInfo i(QString::fromLatin1(zoneinfoPath)+z); return i.exists()&&i.isFile()&&i.isReadable(); }
bool updateTimezone(const QString &z, QString *e) { const QString target=QString::fromLatin1(zoneinfoPath)+z; QFile::remove(localtimePath); if(!QFile::link(target,localtimePath)){*e="Could not update /etc/localtime.";return false;} QSaveFile f(timezonePath); if(!f.open(QIODevice::WriteOnly|QIODevice::Text)||f.write(z.toUtf8()+QByteArray("\n"))<0||!f.commit()){*e="Could not update /etc/timezone.";return false;} QString pe; if(!SystemFilePersistence::persistSymlink(target,localtimePath,&pe)||!SystemFilePersistence::persist(timezonePath,&pe)){*e=QStringLiteral("Timezone changed for this session but could not be persisted: %1").arg(pe);return false;} return true; }
bool clock(qint64 s, QString *e) { timespec t{s,0}; if(::clock_settime(CLOCK_REALTIME,&t)!=0){*e=QStringLiteral("Could not set the system clock: %1").arg(QString::fromLocal8Bit(std::strerror(errno)));return false;} QProcess p; p.start("/sbin/hwclock",{"--systohc"}); if(!p.waitForFinished(10000)||p.exitCode()!=0){*e="System clock changed, but the hardware clock could not be synchronized.";return false;} return true; }
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
KAuth::ActionReply DateTimeHelper::set(const QVariantMap &a) { QString e; const QString op=a.value("operation").toString(); if(op=="timezone"){const QString z=a.value("timezone").toString().trimmed();if(!valid(z))return error("Invalid IANA timezone.",1001);if(!updateTimezone(z,&e))return error(e,1002);}else if(op=="clock"){const qint64 s=a.value("epochSeconds").toLongLong();if(s<0||!clock(s,&e))return error(e.isEmpty()?"Invalid clock value.":e,1003);}else if(op=="hostname"){if(!updateHostname(a.value("hostname").toString().trimmed(),&e))return error(e,1005);}else if(op=="locale"){if(!updateLocale(a.value("locale").toString().trimmed(),&e))return error(e,1004);}else return error("Unknown System operation.",1000);return KAuth::ActionReply::SuccessReply();}
KAUTH_HELPER_MAIN("org.maui.settings.datetime", DateTimeHelper)
