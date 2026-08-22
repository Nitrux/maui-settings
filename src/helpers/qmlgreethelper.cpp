#include "qmlgreethelper.h"
#include "systemfilepersistence.h"

#include <QDebug>
#include <QDir>
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
constexpr auto hyprpaperConfigPath = "/etc/greetd/hyprpaper.conf";
constexpr auto greetdHomePath = "/var/lib/greetd";
constexpr qint64 maximumImageSize = 100 * 1024 * 1024;

QVariantMap readSettings()
{
    QSettings settings(QString::fromLatin1(configPath), QSettings::IniFormat);
    return {
        {QStringLiteral("wallpaperPath"), settings.value(
            QStringLiteral("Appearance/BackgroundImage"),
            QStringLiteral("/usr/share/wallpapers/Blossom/contents/images/4096x2304.png"))},
        {QStringLiteral("iconMode"), settings.value(
            QStringLiteral("Appearance/IconMode"), QStringLiteral("system"))},
        {QStringLiteral("avatarPath"), settings.value(
            QStringLiteral("Appearance/AvatarImage"))},
        {QStringLiteral("timeFormat"), settings.value(
            QStringLiteral("Clock/TimeFormat"), QStringLiteral("hh:mm"))},
        {QStringLiteral("dateFormat"), settings.value(
            QStringLiteral("Clock/DateFormat"), QStringLiteral("dddd, dd MMMM yyyy"))},
        {QStringLiteral("blurEnabled"), settings.value(
            QStringLiteral("Appearance/BlurEnabled"), true)},
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

bool writeHyprpaperWallpaper(const QString &wallpaperPath, QString *error)
{
    const QFileInfo sourceInfo(QString::fromLatin1(hyprpaperConfigPath));
    const QFileDevice::Permissions sourcePermissions = sourceInfo.permissions();
    QFile source(sourceInfo.absoluteFilePath());
    if (!source.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        *error = QStringLiteral("Could not read %1.").arg(sourceInfo.absoluteFilePath());
        return false;
    }

    QStringList lines = QString::fromUtf8(source.readAll()).split(QLatin1Char(10));
    bool inWallpaperBlock = false;
    bool replaced = false;
    for (int i = 0; i < lines.size(); ++i)
    {
        const QString trimmed = lines.at(i).trimmed();
        if (trimmed.startsWith(QStringLiteral("wallpaper"))
            && trimmed.contains(QLatin1Char(123)))
        {
            inWallpaperBlock = true;
            continue;
        }
        if (!inWallpaperBlock)
            continue;
        if (trimmed == QStringLiteral("}"))
            break;
        if (!trimmed.startsWith(QStringLiteral("path"))
            || !trimmed.mid(4).trimmed().startsWith(QLatin1Char(61)))
            continue;

        const qsizetype pathStart = lines.at(i).indexOf(QStringLiteral("path"));
        lines[i] = lines.at(i).left(pathStart)
            + QStringLiteral("path = ") + iniValue(wallpaperPath);
        replaced = true;
        break;
    }

    if (!replaced)
    {
        *error = QStringLiteral("Could not find a wallpaper path in %1.").arg(sourceInfo.absoluteFilePath());
        return false;
    }

    QSaveFile destination(sourceInfo.absoluteFilePath());
    if (!destination.open(QIODevice::WriteOnly | QIODevice::Text)
        || !destination.setPermissions(sourcePermissions)
        || destination.write(lines.join(QLatin1Char(10)).toUtf8()) < 0
        || !destination.commit())
    {
        *error = QStringLiteral("Could not write %1.").arg(sourceInfo.absoluteFilePath());
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

bool copyImageToGreetd(const QString &sourcePath, const QString &targetPath,
                       const QString &label, QString *error)
{
    const QString cleanTargetPath = QDir::cleanPath(targetPath);
    const QString relativeTargetPath = QDir(QString::fromLatin1(greetdHomePath))
        .relativeFilePath(cleanTargetPath);
    if (relativeTargetPath == QStringLiteral("..")
        || relativeTargetPath.startsWith(QStringLiteral("../"))
        || QDir::isAbsolutePath(relativeTargetPath))
    {
        *error = QStringLiteral("The %1 destination must be inside /var/lib/greetd.").arg(label);
        return false;
    }

    const QFileInfo targetInfo(cleanTargetPath);
    if (!QDir().mkpath(targetInfo.absolutePath()))
    {
        *error = QStringLiteral("Could not create the %1 directory %2.")
                     .arg(label)
                     .arg(targetInfo.absolutePath());
        return false;
    }

    const QFileDevice::Permissions directoryPermissions =
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
        | QFileDevice::ReadGroup | QFileDevice::ExeGroup
        | QFileDevice::ReadOther | QFileDevice::ExeOther;
    QString directoryPath = targetInfo.absolutePath();
    const QString greetdHome = QDir::cleanPath(QString::fromLatin1(greetdHomePath));
    while (directoryPath.startsWith(greetdHome + QLatin1Char(47)))
    {
        if (!QFile::setPermissions(directoryPath, directoryPermissions))
        {
            *error = QStringLiteral("Could not make the %1 directory readable by greetd: %2.")
                         .arg(label)
                         .arg(directoryPath);
            return false;
        }
        directoryPath = QFileInfo(directoryPath).absolutePath();
    }

    QFile source(sourcePath);
    if (!source.open(QIODevice::ReadOnly))
    {
        *error = QStringLiteral("Could not read %1.").arg(sourcePath);
        return false;
    }

    QSaveFile destination(cleanTargetPath);
    if (!destination.open(QIODevice::WriteOnly)
        || destination.write(source.readAll()) < 0
        || !destination.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                       | QFileDevice::ReadGroup | QFileDevice::ReadOther)
        || !destination.commit())
    {
        *error = QStringLiteral("Could not copy the %1 to %2.").arg(label).arg(cleanTargetPath);
        return false;
    }

    return true;
}

KAuth::ActionReply QmlGreetHelper::save(const QVariantMap &arguments)
{
    qDebug() << "QmlGreetHelper::save received"
             << "keys=" << arguments.keys()
             << "configPath=" << QString::fromLatin1(configPath);

    QString wallpaperPath;
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

    const bool copyWallpaper = arguments.contains(QStringLiteral("wallpaperSourcePath"));
    if (copyWallpaper)
    {
        const QString destinationPath = boundedString(
            arguments, QStringLiteral("wallpaperPath"), 4096);
        QString sourcePath;
        if (!validateLocalFile(
                boundedString(arguments, QStringLiteral("wallpaperSourcePath"), 4096),
                QStringLiteral("Wallpaper"), true, false, &sourcePath, &validationError)
            || !copyImageToGreetd(sourcePath, destinationPath, QStringLiteral("wallpaper"),
                                  &validationError))
        {
            return helperError(validationError, 1003);
        }
        wallpaperPath = QDir::cleanPath(destinationPath);
    }
    else if (!validateChangedFile(
                   QStringLiteral("wallpaperPath"), QStringLiteral("Appearance/BackgroundImage"),
                   QStringLiteral("Wallpaper"), true, true,
                   QStringLiteral("/usr/share/wallpapers/Blossom/contents/images/4096x2304.png"), &wallpaperPath))
    {
        return helperError(validationError, 1003);
    }

    const bool copyAvatar = arguments.contains(QStringLiteral("avatarSourcePath"));
    if (copyAvatar)
    {
        const QString destinationPath = boundedString(
            arguments, QStringLiteral("avatarPath"), 4096);
        QString sourcePath;
        if (!validateLocalFile(
                boundedString(arguments, QStringLiteral("avatarSourcePath"), 4096),
                QStringLiteral("Avatar"), true, false, &sourcePath, &validationError)
            || !copyImageToGreetd(sourcePath, destinationPath, QStringLiteral("avatar"),
                                  &validationError))
        {
            return helperError(validationError, 1003);
        }
        avatarPath = QDir::cleanPath(destinationPath);
    }
    else if (!validateChangedFile(
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
        {QStringLiteral("Appearance"), QStringLiteral("IconMode"), QStringLiteral("iconMode"), arguments.value(QStringLiteral("iconMode")).toString().trimmed().toLower() == QStringLiteral("nerd") ? QStringLiteral("nerd") : QStringLiteral("system")},
        {QStringLiteral("Appearance"), QStringLiteral("AvatarImage"), QStringLiteral("avatarPath"), avatarPath},
        {QStringLiteral("Appearance"), QStringLiteral("BlurEnabled"), QStringLiteral("blurEnabled"), boolValue(QStringLiteral("blurEnabled"), true)},
        {QStringLiteral("Appearance"), QStringLiteral("OverlayEnabled"), QStringLiteral("overlayEnabled"), boolValue(QStringLiteral("overlayEnabled"), true)},
        {QStringLiteral("Appearance"), QStringLiteral("OverlayOpacity"), QStringLiteral("overlayOpacity"), QString::number(qBound(0.0, arguments.value(QStringLiteral("overlayOpacity"), 0.76).toDouble(), 1.0))},
        {QStringLiteral("Clock"), QStringLiteral("TimeFormat"), QStringLiteral("timeFormat"), boundedString(arguments, QStringLiteral("timeFormat"), 256)},
        {QStringLiteral("Clock"), QStringLiteral("DateFormat"), QStringLiteral("dateFormat"), boundedString(arguments, QStringLiteral("dateFormat"), 256)},
        {QStringLiteral("Behavior"), QStringLiteral("ShowAvatars"), QStringLiteral("showAvatars"), boolValue(QStringLiteral("showAvatars"), true)},
        {QStringLiteral("Behavior"), QStringLiteral("RememberLastUser"), QStringLiteral("rememberLastUser"), boolValue(QStringLiteral("rememberLastUser"), true)},
        {QStringLiteral("Indicators"), QStringLiteral("ShowBattery"), QStringLiteral("showBattery"), boolValue(QStringLiteral("showBattery"), true)},
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

    const bool updateHyprpaper = !wallpaperPath.isEmpty();
    if (updateHyprpaper
        && !writeHyprpaperWallpaper(wallpaperPath, &writeError))
        return helperError(writeError, 1005);

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

    if (updateHyprpaper
        && !SystemFilePersistence::persist(QString::fromLatin1(hyprpaperConfigPath), &persistenceError))
    {
        return helperError(QStringLiteral(
            "QMLGreet settings were applied, but the greeter wallpaper could not be made persistent: %1")
            .arg(persistenceError), 1006);
    }

    const QVariantMap savedSettings = readSettings();
    KAuth::ActionReply reply = KAuth::ActionReply::SuccessReply();
    reply.setData(savedSettings);
    return reply;
}

KAuth::ActionReply QmlGreetHelper::copykdeglobals(const QVariantMap &a)
{
    const QString sourcePath = boundedString(a, QStringLiteral("sourcePath"), 4096);
    const QFileInfo sourceInfo(sourcePath);
    const QString canonicalSource = sourceInfo.canonicalFilePath();
    if (canonicalSource.isEmpty() || sourceInfo.fileName() != QStringLiteral("kdeglobals")
        || QFileInfo(canonicalSource).dir().dirName() != QStringLiteral(".config"))
        return helperError(QStringLiteral("The kdeglobals source must be ~/.config/kdeglobals."), 1101);

    QFile source(canonicalSource);
    if (!source.open(QIODevice::ReadOnly))
        return helperError(QStringLiteral("Could not read %1.").arg(canonicalSource), 1102);

    const QString targetPath = QStringLiteral("/var/lib/greetd/.config/kdeglobals");
    if (!QDir().mkpath(QFileInfo(targetPath).absolutePath()))
        return helperError(QStringLiteral("Could not create the greetd configuration directory."), 1103);

    QSaveFile target(targetPath);
    if (!target.open(QIODevice::WriteOnly)
        || target.write(source.readAll()) < 0
        || !target.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                  | QFileDevice::ReadGroup | QFileDevice::ReadOther)
        || !target.commit())
        return helperError(QStringLiteral("Could not copy kdeglobals to %1.").arg(targetPath), 1104);

    qDebug() << "QmlGreetHelper: copied kdeglobals" << canonicalSource << "to" << targetPath;
    return KAuth::ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.maui.settings.qmlgreet", QmlGreetHelper)
