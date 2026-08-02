#include "datetimehelper.h"
#include "systemfilepersistence.h"
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QSaveFile>
#include <KAuth/HelperSupport>
#include <cerrno>
#include <cstring>
#include <ctime>
namespace {
constexpr auto localtimePath = "/etc/localtime"; constexpr auto timezonePath = "/etc/timezone"; constexpr auto zoneinfoPath = "/usr/share/zoneinfo/";
KAuth::ActionReply error(const QString &message, int code) { auto r=KAuth::ActionReply::HelperErrorReply(); r.setError(code); r.setErrorDescription(message); return r; }
bool valid(const QString &z) { if(z.isEmpty()||z.startsWith('/')||z.contains("..")) return false; QFileInfo i(QString::fromLatin1(zoneinfoPath)+z); return i.exists()&&i.isFile()&&i.isReadable(); }
bool updateTimezone(const QString &z, QString *e) { const QString target=QString::fromLatin1(zoneinfoPath)+z; QFile::remove(localtimePath); if(!QFile::link(target,localtimePath)){*e="Could not update /etc/localtime.";return false;} QSaveFile f(timezonePath); if(!f.open(QIODevice::WriteOnly|QIODevice::Text)||f.write(z.toUtf8()+QByteArray("\n"))<0||!f.commit()){*e="Could not update /etc/timezone.";return false;} QString pe; if(!SystemFilePersistence::persistSymlink(target,localtimePath,&pe)||!SystemFilePersistence::persist(timezonePath,&pe)){*e=QStringLiteral("Timezone changed for this session but could not be persisted: %1").arg(pe);return false;} return true; }
bool clock(qint64 s, QString *e) { timespec t{s,0}; if(::clock_settime(CLOCK_REALTIME,&t)!=0){*e=QStringLiteral("Could not set the system clock: %1").arg(QString::fromLocal8Bit(std::strerror(errno)));return false;} QProcess p; p.start("/sbin/hwclock",{"--systohc"}); if(!p.waitForFinished(10000)||p.exitCode()!=0){*e="System clock changed, but the hardware clock could not be synchronized.";return false;} return true; }
}
KAuth::ActionReply DateTimeHelper::set(const QVariantMap &a) { QString e; const QString op=a.value("operation").toString(); if(op=="timezone"){const QString z=a.value("timezone").toString().trimmed();if(!valid(z))return error("Invalid IANA timezone.",1001);if(!updateTimezone(z,&e))return error(e,1002);}else if(op=="clock"){const qint64 s=a.value("epochSeconds").toLongLong();if(s<0||!clock(s,&e))return error(e.isEmpty()?"Invalid clock value.":e,1003);}else return error("Unknown Date and Time operation.",1000);return KAuth::ActionReply::SuccessReply();}
KAUTH_HELPER_MAIN("org.maui.settings.datetime", DateTimeHelper)
