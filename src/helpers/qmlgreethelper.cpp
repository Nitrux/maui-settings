#include "qmlgreethelper.h"
#include "systemfilepersistence.h"

#include <QDebug>
#include <QFile>
#include <QSaveFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QtGlobal>

#include <KAuth/HelperSupport>

namespace
{
constexpr auto configPath = "/etc/qmlgreet/qmlgreet.conf";
constexpr qint64 maximumImageSize = 100 * 1024 * 1024;

QVariantMap readSettings()
{
    QSettings settings(QString::fromLatin1(configPath), QSettings::IniFormat);
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

KAuth::ActionReply helperError(const QString &message, int errorCode)
{
    qWarning().noquote() << "QmlGreetHelper: error" << errorCode << message;
    KAuth::ActionReply reply = KAuth::ActionReply::HelperErrorReply();
    reply.setError(errorCode);
    reply.setErrorDescription(message);
    return reply;
}

QString boundedString(const QVariantMap &values, const QString &key, qsizetype maximumLength)
{
    return values.value(key).toString().trimmed().left(maximumLength);
}

struct IniValue
{
    QString section;
    QString key;
    QString argument;
    QString value;
};

QString iniValue(const QString &value)
{
    QString result = value;
    result.replace(QLatin1Char(13), QLatin1Char(' '));
    result.replace(QLatin1Char(10), QLatin1Char(' '));
    return result;
}

bool replaceIniValue(QStringList &lines, const IniValue &entry)
{
    int sectionStart = -1;
    int sectionEnd = lines.size();
    const QString header = QStringLiteral("[%1]").arg(entry.section);
    for (int i = 0; i < lines.size(); ++i)
    {
        const QString trimmed = lines.at(i).trimmed();
        if (trimmed == header)
        {
            sectionStart = i;
            continue;
        }
        if (sectionStart >= 0 && trimmed.startsWith(QLatin1Char('[')))
        {
            sectionEnd = i;
            break;
        }
    }
    if (sectionStart < 0)
        return false;

    const QString prefix = entry.key + QLatin1Char('=');
    for (int i = sectionStart + 1; i < sectionEnd; ++i)
    {
        if (lines.at(i).trimmed().startsWith(prefix))
        {
            lines[i] = entry.key + QLatin1Char('=') + iniValue(entry.value);
            return true;
        }
    }
    lines.insert(sectionEnd, entry.key + QLatin1Char('=') + iniValue(entry.value));
    return true;
}

bool writeIniValues(const QString &path, const QList<IniValue> &entries, QString *error)
{
    const QFileInfo sourceInfo(path);
    const QFileDevice::Permissions sourcePermissions = sourceInfo.permissions();
    QFile source(path);
    if (!source.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        *error = QStringLiteral("Could not read %1.").arg(path);
        return false;
    }
    QStringList lines = QString::fromUtf8(source.readAll()).split(QLatin1Char(10));
    for (const IniValue &entry : entries)
    {
        if (replaceIniValue(lines, entry))
            continue;
        if (!lines.isEmpty() && !lines.constLast().trimmed().isEmpty())
            lines.append(QString());
        lines.append(QStringLiteral("[%1]").arg(entry.section));
        lines.append(entry.key + QLatin1Char('=') + iniValue(entry.value));
    }
    QSaveFile destination(path);
    if (!destination.open(QIODevice::WriteOnly | QIODevice::Text)
        || !destination.setPermissions(sourcePermissions)
        || destination.write(lines.join(QLatin1Char(10)).toUtf8()) < 0
        || !destination.commit())
    {
        *error = QStringLiteral("Could not write %1.").arg(path);
        return false;
    }
    return true;
}

bool validateLocalFile(const QString &value,
                       const QString &label,
                       bool image,
                       bool optional,
                       QString *validatedPath,
                       QString *error)
{
    const QString path = value.trimmed();
    if (path.isEmpty())
    {
        if (optional)
        {
            validatedPath->clear();
            return true;
        }

        *error = QStringLiteral("%1 must not be empty.").arg(label);
        return false;
    }

    const QFileInfo suppliedInfo(path);
    if (!suppliedInfo.isAbsolute())
    {
        *error = QStringLiteral("%1 must be an absolute local path.").arg(label);
        return false;
    }

    const QString canonicalPath = suppliedInfo.canonicalFilePath();
    const QFileInfo canonicalInfo(canonicalPath);
    if (canonicalPath.isEmpty() || !canonicalInfo.exists() || !canonicalInfo.isFile()
        || !canonicalInfo.isReadable())
    {
        *error = QStringLiteral("%1 must reference an existing readable regular file.").arg(label);
        return false;
    }

    if (image)
    {
        if (canonicalInfo.size() <= 0 || canonicalInfo.size() > maximumImageSize)
        {
            *error = QStringLiteral("%1 has an invalid size.").arg(label);
            return false;
        }

        const QMimeDatabase database;
        const QString mimeName = database.mimeTypeForFile(
            canonicalPath, QMimeDatabase::MatchContent).name();
        if (!mimeName.startsWith(QStringLiteral("image/")))
        {
            *error = QStringLiteral("%1 is not a recognized image file.").arg(label);
            return false;
        }
    }

    *validatedPath = canonicalPath;
    return true;
}
}

KAuth::ActionReply QmlGreetHelper::save(const QVariantMap &arguments)
{
    qDebug() << "QmlGreetHelper::save received"
             << "keys=" << arguments.keys()
             << "borderRadius=" << arguments.value(QStringLiteral("borderRadius"))
             << "configPath=" << QString::fromLatin1(configPath);

    QString wallpaperPath;
    QString colorSchemePath;
    QString avatarPath;
    QString validationError;

    QSettings currentSettings(QString::fromLatin1(configPath), QSettings::IniFormat);
    currentSettings.sync();
    const auto comparablePath = [](QString path)
    {
        const QUrl url(path);
        if (url.isValid() && url.isLocalFile())
            path = url.toLocalFile();
        return path.trimmed().isEmpty()
            ? QString()
            : QFileInfo(path).absoluteFilePath();
    };

const auto validateChangedFile = [&](const QString &argumentKey,
                                         const QString &configKey,
                                         const QString &label,
                                         bool image,
                                         bool optional,
                                         const QVariant &defaultValue,
                                         QString *validatedPath)
    {
        const QString currentRaw = currentSettings.value(configKey, defaultValue).toString().trimmed();
        const QString requestedRaw = arguments.contains(argumentKey)
            ? boundedString(arguments, argumentKey, 4096) : currentRaw;
        const QString requested = comparablePath(requestedRaw);
        const QString current = comparablePath(currentRaw);
        qDebug() << "QmlGreetHelper: path comparison"
                 << argumentKey << "requestedRaw=" << requestedRaw
                 << "currentRaw=" << currentRaw
                 << "requested=" << requested << "current=" << current;

        // Do not let an existing file reference prevent unrelated settings
        // from being saved. Only a newly submitted path needs validation.
        if (requested == current)
        {
            *validatedPath = requestedRaw;
            return true;
        }

        return validateLocalFile(
            requestedRaw, label, image, optional, validatedPath, &validationError);
    };

    if (!validateChangedFile(
            QStringLiteral("wallpaperPath"), QStringLiteral("Appearance/BackgroundImage"),
            QStringLiteral("Wallpaper"), true, true,
            QStringLiteral("/usr/share/wallpapers/Aqua/contents/images/3840x2160.png"), &wallpaperPath)
        || !validateChangedFile(
            QStringLiteral("colorSchemePath"), QStringLiteral("Appearance/ColorScheme"),
            QStringLiteral("Color scheme"), false, true,
            QStringLiteral("/usr/share/color-schemes/QMLGreetDefault.colors"), &colorSchemePath)
        || !validateChangedFile(
            QStringLiteral("avatarPath"), QStringLiteral("Appearance/AvatarImage"),
            QStringLiteral("Avatar"), true, true,
            QVariant(), &avatarPath))
    {
        return helperError(validationError, 1003);
    }
    
    const auto boolValue = [&](const QString &key, bool fallback)
    {
        return arguments.value(key, fallback).toBool() ? QStringLiteral("true") : QStringLiteral("false");
    };
    const QList<IniValue> entries = {
        {QStringLiteral("General"), QStringLiteral("DefaultSession"), QStringLiteral("defaultSession"), boundedString(arguments, QStringLiteral("defaultSession"), 256)},
        {QStringLiteral("Appearance"), QStringLiteral("BackgroundImage"), QStringLiteral("wallpaperPath"), wallpaperPath},
        {QStringLiteral("Appearance"), QStringLiteral("ColorScheme"), QStringLiteral("colorSchemePath"), colorSchemePath},
        {QStringLiteral("Appearance"), QStringLiteral("IconTheme"), QStringLiteral("iconTheme"), boundedString(arguments, QStringLiteral("iconTheme"), 256)},
        {QStringLiteral("Appearance"), QStringLiteral("Font"), QStringLiteral("fontFamily"), boundedString(arguments, QStringLiteral("fontFamily"), 256)},
        {QStringLiteral("Appearance"), QStringLiteral("FontSize"), QStringLiteral("fontSize"), QString::number(qBound(1, arguments.value(QStringLiteral("fontSize"), 10).toInt(), 256))},
        {QStringLiteral("Style"), QStringLiteral("BorderRadius"), QStringLiteral("borderRadius"), QString::number(qBound(0, arguments.value(QStringLiteral("borderRadius"), 8).toInt(), 256))},
        {QStringLiteral("Appearance"), QStringLiteral("AvatarImage"), QStringLiteral("avatarPath"), avatarPath},
        {QStringLiteral("Appearance"), QStringLiteral("BlurEnabled"), QStringLiteral("blurEnabled"), boolValue(QStringLiteral("blurEnabled"), true)},
        {QStringLiteral("Appearance"), QStringLiteral("OverlayEnabled"), QStringLiteral("overlayEnabled"), boolValue(QStringLiteral("overlayEnabled"), true)},
        {QStringLiteral("Appearance"), QStringLiteral("OverlayOpacity"), QStringLiteral("overlayOpacity"), QString::number(qBound(0.0, arguments.value(QStringLiteral("overlayOpacity"), 0.76).toDouble(), 1.0))},
        {QStringLiteral("Clock"), QStringLiteral("TimeFormat"), QStringLiteral("timeFormat"), boundedString(arguments, QStringLiteral("timeFormat"), 256)},
        {QStringLiteral("Clock"), QStringLiteral("DateFormat"), QStringLiteral("dateFormat"), boundedString(arguments, QStringLiteral("dateFormat"), 256)},
        {QStringLiteral("Behavior"), QStringLiteral("AnimationsEnabled"), QStringLiteral("animationsEnabled"), boolValue(QStringLiteral("animationsEnabled"), true)},
        {QStringLiteral("Behavior"), QStringLiteral("ShowAvatars"), QStringLiteral("showAvatars"), boolValue(QStringLiteral("showAvatars"), true)},
        {QStringLiteral("Behavior"), QStringLiteral("RememberLastUser"), QStringLiteral("rememberLastUser"), boolValue(QStringLiteral("rememberLastUser"), true)},
        {QStringLiteral("Indicators"), QStringLiteral("ShowBattery"), QStringLiteral("showBattery"), boolValue(QStringLiteral("showBattery"), true)},
        {QStringLiteral("Indicators"), QStringLiteral("ShowSystemResources"), QStringLiteral("showSystemResources"), boolValue(QStringLiteral("showSystemResources"), true)},
    };

    QList<IniValue> changedEntries;
    for (const IniValue &entry : entries)
    {
        if (arguments.contains(entry.argument))
            changedEntries.append(entry);
    }

    QString writeError;
    if (!writeIniValues(QString::fromLatin1(configPath), changedEntries, &writeError))
        return helperError(writeError, 1001);

    QSettings liveVerification(QString::fromLatin1(configPath), QSettings::IniFormat);
    liveVerification.sync();
    if (liveVerification.status() != QSettings::NoError)
        return helperError(QStringLiteral("Could not verify /etc/qmlgreet/qmlgreet.conf after writing."), 1002);

    qDebug() << "QmlGreetHelper: live settings write succeeded; persisting lower file";
    QString persistenceError;
    if (!SystemFilePersistence::persist(QString::fromLatin1(configPath), &persistenceError))
    {
        return helperError(QStringLiteral(
            "QMLGreet settings were applied to the running system, but could not be made persistent: %1")
            .arg(persistenceError), 1004);
    }

    const QVariantMap savedSettings = readSettings();
    qDebug() << "QmlGreetHelper: save succeeded"
             << "returned borderRadius="
             << savedSettings.value(QStringLiteral("borderRadius"));
    KAuth::ActionReply reply = KAuth::ActionReply::SuccessReply();
    reply.setData(savedSettings);
    return reply;
}

KAUTH_HELPER_MAIN("org.maui.settings.qmlgreet", QmlGreetHelper)
