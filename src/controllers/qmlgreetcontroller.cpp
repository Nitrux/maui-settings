#include "qmlgreetcontroller.h"

#include <QDir>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDebug>
#include <QFileInfo>
#include <QGuiApplication>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
#include <QWindow>
#include <QtGlobal>

#include <KAuth/Action>
#include <KAuth/ActionReply>
#include <KAuth/ExecuteJob>
#include <KJob>

namespace
{
constexpr auto systemConfigPath = "/etc/qmlgreet/qmlgreet.conf";
constexpr auto helperId = "org.maui.settings.qmlgreet";
constexpr auto saveActionId = "org.maui.settings.qmlgreet.save";

QVariantMap readSettings(QSettings &settings)
{
    return {
        {QStringLiteral("wallpaperPath"), settings.value(
            QStringLiteral("Appearance/BackgroundImage"),
            QStringLiteral("/usr/share/wallpapers/Aqua/contents/images/3840x2160.png"))},
        {QStringLiteral("colorSchemePath"), settings.value(
            QStringLiteral("Appearance/ColorScheme"),
            QStringLiteral("/usr/share/color-schemes/QMLGreetDefault.colors"))},
        {QStringLiteral("iconTheme"), settings.value(
            QStringLiteral("Appearance/IconTheme"), QStringLiteral("hicolor"))},
        {QStringLiteral("fontFamily"), settings.value(
            QStringLiteral("Appearance/Font"), QStringLiteral("Noto Sans"))},
        {QStringLiteral("fontSize"), settings.value(
            QStringLiteral("Appearance/FontSize"), 10)},
        {QStringLiteral("borderRadius"), settings.value(
            QStringLiteral("Style/BorderRadius"), 8)},
        {QStringLiteral("avatarPath"), settings.value(
            QStringLiteral("Appearance/AvatarImage"))},
        {QStringLiteral("timeFormat"), settings.value(
            QStringLiteral("Clock/TimeFormat"), QStringLiteral("hh:mm"))},
        {QStringLiteral("dateFormat"), settings.value(
            QStringLiteral("Clock/DateFormat"), QStringLiteral("dddd, dd MMMM yyyy"))},
        {QStringLiteral("blurEnabled"), settings.value(
            QStringLiteral("Appearance/BlurEnabled"), true)},
        {QStringLiteral("animationsEnabled"), settings.value(
            QStringLiteral("Behavior/AnimationsEnabled"), true)},
        {QStringLiteral("overlayEnabled"), settings.value(
            QStringLiteral("Appearance/OverlayEnabled"), true)},
        {QStringLiteral("overlayOpacity"), settings.value(
            QStringLiteral("Appearance/OverlayOpacity"), 0.76)},
        {QStringLiteral("defaultSession"), settings.value(QStringLiteral("DefaultSession"))},
        {QStringLiteral("showAvatars"), settings.value(
            QStringLiteral("Behavior/ShowAvatars"), true)},
        {QStringLiteral("rememberLastUser"), settings.value(
            QStringLiteral("Behavior/RememberLastUser"), true)},
        {QStringLiteral("showBattery"), settings.value(
            QStringLiteral("Indicators/ShowBattery"), true)},
        {QStringLiteral("showSystemResources"), settings.value(
            QStringLiteral("Indicators/ShowSystemResources"), true)},
    };
}
}

QmlGreetController::QmlGreetController(QObject *parent)
    : QObject(parent)
{
    refreshSessions();
    reload();
}

QString QmlGreetController::configPath() const { return QString::fromLatin1(systemConfigPath); }
bool QmlGreetController::saveAvailable() const { return m_saveAvailable; }

QString QmlGreetController::wallpaperDirectory() const
{
    return m_wallpaperPath.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
        : QFileInfo(m_wallpaperPath).absolutePath();
}

QString QmlGreetController::wallpaperPath() const { return m_wallpaperPath; }

QString QmlGreetController::colorSchemeDirectory() const
{
    return m_colorSchemePath.isEmpty()
        ? QStringLiteral("/usr/share/color-schemes")
        : QFileInfo(m_colorSchemePath).absolutePath();
}

QString QmlGreetController::colorSchemePath() const { return m_colorSchemePath; }
QString QmlGreetController::iconTheme() const { return m_iconTheme; }
QString QmlGreetController::fontFamily() const { return m_fontFamily; }
int QmlGreetController::fontSize() const { return m_fontSize; }
int QmlGreetController::borderRadius() const { return m_borderRadius; }

QString QmlGreetController::avatarDirectory() const
{
    return m_avatarPath.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
        : QFileInfo(m_avatarPath).absolutePath();
}

QString QmlGreetController::avatarPath() const { return m_avatarPath; }
QString QmlGreetController::timeFormat() const { return m_timeFormat; }
QString QmlGreetController::dateFormat() const { return m_dateFormat; }
bool QmlGreetController::blurEnabled() const { return m_blurEnabled; }
bool QmlGreetController::animationsEnabled() const { return m_animationsEnabled; }
bool QmlGreetController::overlayEnabled() const { return m_overlayEnabled; }
double QmlGreetController::overlayOpacity() const { return m_overlayOpacity; }
QStringList QmlGreetController::availableSessions() const { return m_availableSessions; }
QString QmlGreetController::defaultSession() const { return m_defaultSession; }
bool QmlGreetController::showAvatars() const { return m_showAvatars; }
bool QmlGreetController::rememberLastUser() const { return m_rememberLastUser; }
bool QmlGreetController::showBattery() const { return m_showBattery; }
bool QmlGreetController::showSystemResources() const { return m_showSystemResources; }
bool QmlGreetController::dirty() const { return m_dirty; }
bool QmlGreetController::loading() const { return m_loading; }
bool QmlGreetController::saving() const { return m_saving; }
QString QmlGreetController::statusMessage() const { return m_statusMessage; }
QString QmlGreetController::errorMessage() const { return m_errorMessage; }

QString QmlGreetController::normalizeLocalPath(const QString &value)
{
    QString path = value.trimmed();
    if (path.startsWith(QStringLiteral("file:")))
        path = QUrl(path).toLocalFile();

    return path.isEmpty() ? QString() : QFileInfo(path).absoluteFilePath();
}

#define SET_STRING_SETTING(Method, Member, Signal, NormalizedValue) \
    void QmlGreetController::Method(const QString &value) \
    { \
        const QString normalized = (NormalizedValue); \
        if (Member == normalized) \
            return; \
        Member = normalized; \
        Q_EMIT Signal(); \
        updateDirty(); \
    }

SET_STRING_SETTING(setWallpaperPath, m_wallpaperPath, wallpaperPathChanged, normalizeLocalPath(value))
SET_STRING_SETTING(setColorSchemePath, m_colorSchemePath, colorSchemePathChanged, normalizeLocalPath(value))
SET_STRING_SETTING(setIconTheme, m_iconTheme, iconThemeChanged, value.trimmed())
SET_STRING_SETTING(setFontFamily, m_fontFamily, fontFamilyChanged, value.trimmed())
SET_STRING_SETTING(setAvatarPath, m_avatarPath, avatarPathChanged, normalizeLocalPath(value))
SET_STRING_SETTING(setTimeFormat, m_timeFormat, timeFormatChanged,
                   value.trimmed().isEmpty() ? QStringLiteral("hh:mm") : value.trimmed())
SET_STRING_SETTING(setDateFormat, m_dateFormat, dateFormatChanged,
                   value.trimmed().isEmpty() ? QStringLiteral("dddd, dd MMMM yyyy") : value.trimmed())
SET_STRING_SETTING(setDefaultSession, m_defaultSession, defaultSessionChanged, value.trimmed())

#undef SET_STRING_SETTING

#define SET_BOOLEAN_SETTING(Method, Member, Signal) \
    void QmlGreetController::Method(bool value) \
    { \
        if (Member == value) \
            return; \
        Member = value; \
        Q_EMIT Signal(); \
        updateDirty(); \
    }

SET_BOOLEAN_SETTING(setBlurEnabled, m_blurEnabled, blurEnabledChanged)
SET_BOOLEAN_SETTING(setAnimationsEnabled, m_animationsEnabled, animationsEnabledChanged)
SET_BOOLEAN_SETTING(setOverlayEnabled, m_overlayEnabled, overlayEnabledChanged)
SET_BOOLEAN_SETTING(setShowAvatars, m_showAvatars, showAvatarsChanged)
SET_BOOLEAN_SETTING(setRememberLastUser, m_rememberLastUser, rememberLastUserChanged)
SET_BOOLEAN_SETTING(setShowBattery, m_showBattery, showBatteryChanged)
SET_BOOLEAN_SETTING(setShowSystemResources, m_showSystemResources, showSystemResourcesChanged)

#undef SET_BOOLEAN_SETTING

void QmlGreetController::setFontSize(int value)
{
    value = qBound(1, value, 256);
    if (m_fontSize == value)
        return;

    m_fontSize = value;
    Q_EMIT fontSizeChanged();
    updateDirty();
}

void QmlGreetController::setBorderRadius(int value)
{
    value = qBound(0, value, 256);
    if (m_borderRadius == value)
        return;

    qDebug() << "QmlGreetController: borderRadius changed"
             << m_borderRadius << "->" << value;
    m_borderRadius = value;
    Q_EMIT borderRadiusChanged();
    updateDirty();
}

void QmlGreetController::setOverlayOpacity(double value)
{
    value = qBound(0.0, value, 1.0);
    if (qFuzzyCompare(m_overlayOpacity, value))
        return;

    m_overlayOpacity = value;
    Q_EMIT overlayOpacityChanged();
    updateDirty();
}

QVariantMap QmlGreetController::stagedValues() const
{
    return {
        {QStringLiteral("wallpaperPath"), m_wallpaperPath},
        {QStringLiteral("colorSchemePath"), m_colorSchemePath},
        {QStringLiteral("iconTheme"), m_iconTheme},
        {QStringLiteral("fontFamily"), m_fontFamily},
        {QStringLiteral("fontSize"), m_fontSize},
        {QStringLiteral("borderRadius"), m_borderRadius},
        {QStringLiteral("avatarPath"), m_avatarPath},
        {QStringLiteral("timeFormat"), m_timeFormat},
        {QStringLiteral("dateFormat"), m_dateFormat},
        {QStringLiteral("blurEnabled"), m_blurEnabled},
        {QStringLiteral("animationsEnabled"), m_animationsEnabled},
        {QStringLiteral("overlayEnabled"), m_overlayEnabled},
        {QStringLiteral("overlayOpacity"), m_overlayOpacity},
        {QStringLiteral("defaultSession"), m_defaultSession},
        {QStringLiteral("showAvatars"), m_showAvatars},
        {QStringLiteral("rememberLastUser"), m_rememberLastUser},
        {QStringLiteral("showBattery"), m_showBattery},
        {QStringLiteral("showSystemResources"), m_showSystemResources},
    };
}

QVariantMap QmlGreetController::changedValues() const
{
    const QVariantMap staged = stagedValues();
    QVariantMap changed;
    for (auto it = staged.cbegin(); it != staged.cend(); ++it)
    {
        if (!m_savedValues.contains(it.key()) || m_savedValues.value(it.key()) != it.value())
            changed.insert(it.key(), it.value());
    }
    return changed;
}

void QmlGreetController::applyValues(const QVariantMap &values)
{
    m_wallpaperPath = normalizeLocalPath(values.value(
        QStringLiteral("wallpaperPath"),
        QStringLiteral("/usr/share/wallpapers/Aqua/contents/images/3840x2160.png")).toString());
    m_colorSchemePath = normalizeLocalPath(values.value(
        QStringLiteral("colorSchemePath"),
        QStringLiteral("/usr/share/color-schemes/QMLGreetDefault.colors")).toString());
    m_iconTheme = values.value(
        QStringLiteral("iconTheme"), QStringLiteral("hicolor")).toString().trimmed();
    m_fontFamily = values.value(
        QStringLiteral("fontFamily"), QStringLiteral("Noto Sans")).toString().trimmed();
    m_fontSize = qBound(1, values.value(QStringLiteral("fontSize"), 10).toInt(), 256);
    m_borderRadius = qBound(
        0, values.value(QStringLiteral("borderRadius"), 8).toInt(), 256);
    m_avatarPath = normalizeLocalPath(values.value(QStringLiteral("avatarPath")).toString());
    m_timeFormat = values.value(QStringLiteral("timeFormat"), QStringLiteral("hh:mm")).toString();
    m_dateFormat = values.value(
        QStringLiteral("dateFormat"), QStringLiteral("dddd, dd MMMM yyyy")).toString();
    m_blurEnabled = values.value(QStringLiteral("blurEnabled"), true).toBool();
    m_animationsEnabled = values.value(QStringLiteral("animationsEnabled"), true).toBool();
    m_overlayEnabled = values.value(QStringLiteral("overlayEnabled"), true).toBool();
    m_overlayOpacity = qBound(
        0.0, values.value(QStringLiteral("overlayOpacity"), 0.76).toDouble(), 1.0);
    m_defaultSession = values.value(QStringLiteral("defaultSession")).toString().trimmed();
    m_showAvatars = values.value(QStringLiteral("showAvatars"), true).toBool();
    m_rememberLastUser = values.value(QStringLiteral("rememberLastUser"), true).toBool();
    m_showBattery = values.value(QStringLiteral("showBattery"), true).toBool();
    m_showSystemResources = values.value(
        QStringLiteral("showSystemResources"), true).toBool();

    Q_EMIT wallpaperPathChanged();
    Q_EMIT colorSchemePathChanged();
    Q_EMIT iconThemeChanged();
    Q_EMIT fontFamilyChanged();
    Q_EMIT fontSizeChanged();
    Q_EMIT borderRadiusChanged();
    Q_EMIT avatarPathChanged();
    Q_EMIT timeFormatChanged();
    Q_EMIT dateFormatChanged();
    Q_EMIT blurEnabledChanged();
    Q_EMIT animationsEnabledChanged();
    Q_EMIT overlayEnabledChanged();
    Q_EMIT overlayOpacityChanged();
    Q_EMIT defaultSessionChanged();
    Q_EMIT showAvatarsChanged();
    Q_EMIT rememberLastUserChanged();
    Q_EMIT showBatteryChanged();
    Q_EMIT showSystemResourcesChanged();
}

void QmlGreetController::updateDirty()
{
    const bool value = stagedValues() != m_savedValues;
    if (m_dirty == value)
        return;

    m_dirty = value;
    Q_EMIT dirtyChanged();
    setStatusMessage(value ? QStringLiteral("Changes are waiting to be saved.") : QString());
    if (value)
        setErrorMessage(QString());
}

void QmlGreetController::refreshSaveAvailability()
{
    KAuth::Action action(QString::fromLatin1(saveActionId));
    action.setHelperId(QString::fromLatin1(helperId));

    const QString serviceName = QString::fromLatin1(helperId);
    QDBusConnectionInterface *busInterface = QDBusConnection::systemBus().interface();
    const QDBusReply<bool> registered = busInterface
        ? busInterface->isServiceRegistered(serviceName) : QDBusReply<bool>();
    const QDBusReply<QStringList> activatable = busInterface
        ? busInterface->activatableServiceNames() : QDBusReply<QStringList>();
    const bool helperAvailable = (registered.isValid() && registered.value())
        || (activatable.isValid() && activatable.value().contains(serviceName));
    const bool available = action.isValid() && helperAvailable;
    qDebug() << "QmlGreetController: save availability"
             << "actionValid=" << action.isValid()
             << "registered=" << (registered.isValid() && registered.value())
             << "activatable=" << (activatable.isValid()
                    && activatable.value().contains(serviceName))
             << "available=" << available;
    if (m_saveAvailable == available)
        return;

    m_saveAvailable = available;
    Q_EMIT saveAvailableChanged();
}

void QmlGreetController::setLoading(bool value)
{
    if (m_loading == value)
        return;
    m_loading = value;
    Q_EMIT loadingChanged();
}

void QmlGreetController::setSaving(bool value)
{
    if (m_saving == value)
        return;
    m_saving = value;
    Q_EMIT savingChanged();
}

void QmlGreetController::setStatusMessage(const QString &value)
{
    if (m_statusMessage == value)
        return;
    m_statusMessage = value;
    Q_EMIT statusMessageChanged();
}

void QmlGreetController::setErrorMessage(const QString &value)
{
    if (m_errorMessage == value)
        return;
    m_errorMessage = value;
    Q_EMIT errorMessageChanged();
}

QString QmlGreetController::authorizationError(int errorCode, const QString &fallback)
{
    switch (errorCode)
    {
    case KAuth::ActionReply::UserCancelledError:
        return QStringLiteral("Authentication was cancelled.");
    case KAuth::ActionReply::AuthorizationDeniedError:
        return QStringLiteral("Authorization to modify QMLGreet settings was denied.");
    case KAuth::ActionReply::NoResponderError:
        return QStringLiteral("The privileged QMLGreet settings helper is unavailable.");
    case KAuth::ActionReply::NoSuchActionError:
    case KAuth::ActionReply::InvalidActionError:
        return QStringLiteral("The QMLGreet PolicyKit action is not installed correctly.");
    case KAuth::ActionReply::HelperBusyError:
        return QStringLiteral("The privileged QMLGreet settings helper is busy.");
    default:
        return fallback.isEmpty()
            ? QStringLiteral("The privileged QMLGreet settings operation failed.") : fallback;
    }
}

void QmlGreetController::reload()
{
    if (m_loading || m_saving)
        return;

    refreshSessions();
    refreshSaveAvailability();
    setLoading(true);
    setErrorMessage(QString());
    setStatusMessage(QStringLiteral("Loading system-wide QMLGreet settings…"));

    const QFileInfo configFile(QString::fromLatin1(systemConfigPath));
    if (!configFile.exists() || !configFile.isFile() || !configFile.isReadable())
    {
        setLoading(false);
        const QString message = QStringLiteral(
            "Could not read /etc/qmlgreet/qmlgreet.conf. Check that the file exists and is readable.");
        setStatusMessage(QString());
        setErrorMessage(message);
        Q_EMIT reloadFailed(message);
        return;
    }

    QSettings settings(configFile.absoluteFilePath(), QSettings::IniFormat);
    settings.sync();
    if (settings.status() != QSettings::NoError)
    {
        setLoading(false);
        const QString message = settings.status() == QSettings::FormatError
            ? QStringLiteral("/etc/qmlgreet/qmlgreet.conf is not a valid INI file.")
            : QStringLiteral("Could not read /etc/qmlgreet/qmlgreet.conf.");
        setStatusMessage(QString());
        setErrorMessage(message);
        Q_EMIT reloadFailed(message);
        return;
    }

    applyValues(readSettings(settings));
    m_savedValues = stagedValues();
    if (m_dirty)
    {
        m_dirty = false;
        Q_EMIT dirtyChanged();
    }
    setLoading(false);
    setErrorMessage(QString());
    setStatusMessage(QString());
}

bool QmlGreetController::save()
{
    qDebug() << "QmlGreetController::save"
             << "loading=" << m_loading
             << "saving=" << m_saving
             << "saveAvailable=" << m_saveAvailable
             << "dirty=" << m_dirty
             << "borderRadius=" << m_borderRadius;
    if (m_loading || m_saving || !m_saveAvailable)
    {
        qWarning() << "QmlGreetController: save rejected by state guard";
        return false;
    }
    if (!m_dirty)
    {
        setStatusMessage(QStringLiteral("No changes to save."));
        return true;
    }

    const QVariantMap values = changedValues();
    qDebug() << "QmlGreetController: executing KAuth save"
             << "keys=" << values.keys()
             << "borderRadius argument=" << values.value(QStringLiteral("borderRadius"));
    setSaving(true);
    setErrorMessage(QString());
    setStatusMessage(QStringLiteral("Waiting for administrator authentication…"));

    KAuth::Action action(QString::fromLatin1(saveActionId));
    action.setHelperId(QString::fromLatin1(helperId));
    action.setArguments(values);
    if (QWindow *window = QGuiApplication::focusWindow())
        action.setParentWindow(window);

    KAuth::ExecuteJob *job = action.execute();
    connect(job, &KJob::result, this, [this, job](KJob *)
    {
        setSaving(false);
        qDebug() << "QmlGreetController: KAuth save completed"
                 << "error=" << job->error()
                 << "errorText=" << job->errorText()
                 << "returned borderRadius="
                 << job->data().value(QStringLiteral("borderRadius"));
        if (job->error() != 0)
        {
            const bool cancelled = job->error() == KAuth::ActionReply::UserCancelledError;
            const QString message = authorizationError(job->error(), job->errorText());
            setStatusMessage(QString());
            setErrorMessage(message);
            Q_EMIT saveFailed(message, cancelled);
            return;
        }

        applyValues(job->data());
        m_savedValues = stagedValues();
        updateDirty();
        setErrorMessage(QString());
        setStatusMessage(m_dirty
            ? QStringLiteral("Settings saved; additional changes are waiting to be saved.")
            : QStringLiteral("System-wide QMLGreet settings saved."));
        Q_EMIT configurationChanged(QStringLiteral("*"));
        Q_EMIT saveSucceeded();
    });
    job->start();
    return true;
}

void QmlGreetController::refreshSessions()
{
    QStringList sessions;
    QSet<QString> seen;
    const QStringList dataDirectories = qEnvironmentVariable(
        "XDG_DATA_DIRS", "/usr/local/share:/usr/share").split(QLatin1Char(':'), Qt::SkipEmptyParts);

    for (const QString &baseDirectory : dataDirectories)
    {
        QDir directory(baseDirectory + QStringLiteral("/wayland-sessions"));
        directory.setNameFilters({QStringLiteral("*.desktop")});

        for (const QString &fileName : directory.entryList(QDir::Files, QDir::Name))
        {
            QSettings desktopFile(directory.absoluteFilePath(fileName), QSettings::IniFormat);
            desktopFile.beginGroup(QStringLiteral("Desktop Entry"));
            const QString name = desktopFile.value(QStringLiteral("Name")).toString().trimmed();
            const QString command = desktopFile.value(QStringLiteral("Exec")).toString().trimmed();
            const bool hidden = desktopFile.value(QStringLiteral("NoDisplay"), false).toBool()
                || desktopFile.value(QStringLiteral("Hidden"), false).toBool();

            if (!hidden && !name.isEmpty() && !command.isEmpty() && !seen.contains(name))
            {
                seen.insert(name);
                sessions.append(name);
            }
        }
    }

    if (m_availableSessions == sessions)
        return;

    m_availableSessions = sessions;
    Q_EMIT availableSessionsChanged();
}
