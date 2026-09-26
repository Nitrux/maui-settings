#include "graphicsmodehelper.h"

#include <QFileInfo>
#include <QProcess>
#include <QVariant>

#include <KAuth/HelperSupport>

namespace
{
constexpr auto overlayrootChrootPath = "/usr/sbin/overlayroot-chroot";
constexpr int processStartTimeoutMs = 5000;
constexpr int processFinishTimeoutMs = 300000;

KAuth::ActionReply helperError(const QString &message, int code)
{
    KAuth::ActionReply reply = KAuth::ActionReply::HelperErrorReply();
    reply.setError(code);
    reply.setErrorDescription(message);
    return reply;
}

bool supportedMode(const QString &mode)
{
    return mode == QStringLiteral("integrated")
        || mode == QStringLiteral("hybrid")
        || mode == QStringLiteral("nvidia");
}
}

KAuth::ActionReply GraphicsModeHelper::set(const QVariantMap &arguments)
{
    const QString mode = arguments.value(QStringLiteral("mode")).toString().trimmed().toLower();
    if (!supportedMode(mode))
        return helperError(QStringLiteral("The selected graphics mode is invalid."), 1001);

    if (!QFileInfo::exists(QString::fromLatin1(overlayrootChrootPath)))
        return helperError(QStringLiteral("overlayroot-chroot is not available."), 1002);

    const QString orchestration = QStringLiteral(
        "/usr/bin/mount -t devtmpfs dev /dev || exit 1; "
        "device=$(/usr/sbin/findfs LABEL=NX_VAR_LIB 2>/dev/null) || exit 1; "
        "[ -n \"$device\" ] || exit 1; "
        "/usr/bin/mount -t auto \"$device\" /var/lib || exit 1; "
        "/usr/bin/envycontrol --switch \"$1\"; "
        "envycontrol_status=$?; "
        "/usr/bin/sync; "
        "sync_status=$?; "
        "[ $envycontrol_status -eq 0 ] || exit $envycontrol_status; "
        "exit $sync_status");

    QProcess process;
    process.setProgram(QString::fromLatin1(overlayrootChrootPath));
    process.setArguments({
        QStringLiteral("/bin/sh"),
        QStringLiteral("-c"),
        orchestration,
        QStringLiteral("maui-settings-graphics"),
        mode,
    });
    process.start();

    if (!process.waitForStarted(processStartTimeoutMs))
        return helperError(QStringLiteral("Could not start overlayroot-chroot: %1").arg(process.errorString()), 1003);

    if (!process.waitForFinished(processFinishTimeoutMs))
    {
        process.kill();
        process.waitForFinished();
        return helperError(QStringLiteral("The graphics mode operation timed out."), 1004);
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
    {
        const QString error = QString::fromUtf8(process.readAllStandardError()).trimmed();
        return helperError(error.isEmpty()
            ? QStringLiteral("The graphics mode operation failed.")
            : error, 1005);
    }

    return KAuth::ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.maui.settings.graphics", GraphicsModeHelper)
