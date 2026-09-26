#include "graphicsmodecontroller.h"

#include <QFileInfo>
#include <QGuiApplication>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QWindow>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>

namespace
{
constexpr auto actionId = "org.maui.settings.graphics.set";
constexpr auto helperId = "org.maui.settings.graphics";
constexpr auto overlayrootChrootPath = "/usr/sbin/overlayroot-chroot";
constexpr int queryTimeoutMs = 3000;

bool supportedMode(const QString &mode)
{
    return mode == QStringLiteral("integrated")
        || mode == QStringLiteral("hybrid")
        || mode == QStringLiteral("nvidia");
}
}

GraphicsModeController::GraphicsModeController(QObject *parent)
    : QObject(parent)
{
    reload();
}

bool GraphicsModeController::available() const
{
    return m_available;
}

QString GraphicsModeController::currentMode() const
{
    return m_currentMode;
}

QString GraphicsModeController::pendingMode() const
{
    return m_pendingMode;
}

bool GraphicsModeController::rebootRequired() const
{
    return m_rebootRequired;
}

bool GraphicsModeController::busy() const
{
    return m_busy;
}

QString GraphicsModeController::errorMessage() const
{
    return m_errorMessage;
}

void GraphicsModeController::reload()
{
    if (m_busy)
        return;

    m_available = !QStandardPaths::findExecutable(QStringLiteral("envycontrol")).isEmpty()
        && QFileInfo::exists(QString::fromLatin1(overlayrootChrootPath))
        && detectSupportedHardware();
    m_currentMode = m_available ? queryCurrentMode() : QString();
    m_pendingMode.clear();
    m_rebootRequired = false;
    setErrorMessage(QString());
    Q_EMIT stateChanged();
}

void GraphicsModeController::setMode(const QString &mode)
{
    const QString normalizedMode = mode.trimmed().toLower();
    if (!supportedMode(normalizedMode))
    {
        setErrorMessage(QStringLiteral("The selected graphics mode is not supported."));
        return;
    }

    if (!m_available)
    {
        setErrorMessage(QStringLiteral("Graphics mode switching is not available on this system."));
        return;
    }

    if (m_busy)
        return;

    if ((normalizedMode == m_currentMode && !m_rebootRequired)
        || (normalizedMode == m_pendingMode && m_rebootRequired))
        return;

    setBusy(true);
    setErrorMessage(QString());

    KAuth::Action action(QString::fromLatin1(actionId));
    action.setHelperId(QString::fromLatin1(helperId));
    action.setArguments({{QStringLiteral("mode"), normalizedMode}});
    if (QWindow *window = QGuiApplication::focusWindow())
        action.setParentWindow(window);

    KAuth::ExecuteJob *job = action.execute();
    connect(job, &KJob::result, this, [this, job, normalizedMode]
    {
        setBusy(false);
        if (job->error() != 0)
        {
            setErrorMessage(job->errorText().isEmpty()
                ? QStringLiteral("The graphics mode could not be changed.")
                : job->errorText());
            return;
        }

        m_pendingMode = normalizedMode;
        m_rebootRequired = true;
        setErrorMessage(QString());
        Q_EMIT stateChanged();
    });
    job->start();
}

bool GraphicsModeController::detectSupportedHardware() const
{
    const QString lspci = QStandardPaths::findExecutable(QStringLiteral("lspci"));
    if (lspci.isEmpty())
        return false;

    QProcess process;
    process.start(lspci, {QStringLiteral("-Dnn")});
    if (!process.waitForFinished(queryTimeoutMs)
        || process.exitStatus() != QProcess::NormalExit
        || process.exitCode() != 0)
        return false;

    bool nvidiaFound = false;
    bool otherGraphicsFound = false;
    const QRegularExpression graphicsClass(
        QStringLiteral("(VGA compatible controller|3D controller|Display controller)"),
        QRegularExpression::CaseInsensitiveOption);
    const QStringList lines = QString::fromUtf8(process.readAllStandardOutput())
        .split(QRegularExpression(QStringLiteral("\\r?\\n")), Qt::SkipEmptyParts);

    for (const QString &line : lines)
    {
        if (!graphicsClass.match(line).hasMatch())
            continue;

        if (line.contains(QStringLiteral("NVIDIA"), Qt::CaseInsensitive))
            nvidiaFound = true;
        else
            otherGraphicsFound = true;
    }

    if (nvidiaFound && otherGraphicsFound)
        return true;

    return !nvidiaFound && otherGraphicsFound && queryCurrentMode() == QStringLiteral("integrated");
}

QString GraphicsModeController::queryCurrentMode() const
{
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("envycontrol"));
    if (executable.isEmpty())
        return {};

    QProcess process;
    process.start(executable, {QStringLiteral("--query")});
    if (!process.waitForFinished(queryTimeoutMs)
        || process.exitStatus() != QProcess::NormalExit
        || process.exitCode() != 0)
        return {};

    const QString mode = QString::fromUtf8(process.readAllStandardOutput()).trimmed().toLower();
    return supportedMode(mode) ? mode : QString();
}

void GraphicsModeController::setBusy(bool value)
{
    if (m_busy == value)
        return;

    m_busy = value;
    Q_EMIT stateChanged();
}

void GraphicsModeController::setErrorMessage(const QString &message)
{
    if (m_errorMessage == message)
        return;

    m_errorMessage = message;
    Q_EMIT stateChanged();
}
