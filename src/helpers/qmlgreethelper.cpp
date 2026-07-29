// Copyright 2026 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qmlgreethelper.h"

#include <QSettings>
#include <QString>
#include <QtGlobal>

#include <KAuth/HelperSupport>

namespace
{
constexpr auto configPath = "/etc/qmlgreet/qmlgreet.conf";

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
        {QStringLiteral("avatarPath"), settings.value(
            QStringLiteral("Appearance/AvatarImage"))},
        {QStringLiteral("timeFormat"), settings.value(
            QStringLiteral("Clock/TimeFormat"), QStringLiteral("hh:mm"))},
        {QStringLiteral("dateFormat"), settings.value(
            QStringLiteral("Clock/DateFormat"), QStringLiteral("dddd, d MMMM yyyy"))},
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
    KAuth::ActionReply reply = KAuth::ActionReply::HelperErrorReply();
    reply.setError(errorCode);
    reply.setErrorDescription(message);
    return reply;
}

QString boundedString(const QVariantMap &values, const QString &key, qsizetype maximumLength)
{
    return values.value(key).toString().trimmed().left(maximumLength);
}
}

KAuth::ActionReply QmlGreetHelper::save(const QVariantMap &arguments)
{
    QSettings settings(QString::fromLatin1(configPath), QSettings::IniFormat);
    settings.setAtomicSyncRequired(true);
    settings.setValue(QStringLiteral("DefaultSession"), boundedString(
        arguments, QStringLiteral("defaultSession"), 256));
    settings.setValue(QStringLiteral("Appearance/BackgroundImage"), boundedString(
        arguments, QStringLiteral("wallpaperPath"), 4096));
    settings.setValue(QStringLiteral("Appearance/ColorScheme"), boundedString(
        arguments, QStringLiteral("colorSchemePath"), 4096));
    settings.setValue(QStringLiteral("Appearance/IconTheme"), boundedString(
        arguments, QStringLiteral("iconTheme"), 256));
    settings.setValue(QStringLiteral("Appearance/Font"), boundedString(
        arguments, QStringLiteral("fontFamily"), 256));
    settings.setValue(QStringLiteral("Appearance/FontSize"), qBound(
        1, arguments.value(QStringLiteral("fontSize"), 10).toInt(), 256));
    settings.setValue(QStringLiteral("Appearance/AvatarImage"), boundedString(
        arguments, QStringLiteral("avatarPath"), 4096));
    settings.setValue(QStringLiteral("Appearance/BlurEnabled"), arguments.value(
        QStringLiteral("blurEnabled"), true).toBool());
    settings.setValue(QStringLiteral("Appearance/OverlayEnabled"), arguments.value(
        QStringLiteral("overlayEnabled"), true).toBool());
    settings.setValue(QStringLiteral("Appearance/OverlayOpacity"), qBound(
        0.0, arguments.value(QStringLiteral("overlayOpacity"), 0.76).toDouble(), 1.0));
    settings.setValue(QStringLiteral("Clock/TimeFormat"), boundedString(
        arguments, QStringLiteral("timeFormat"), 256));
    settings.setValue(QStringLiteral("Clock/DateFormat"), boundedString(
        arguments, QStringLiteral("dateFormat"), 256));
    settings.setValue(QStringLiteral("Behavior/AnimationsEnabled"), arguments.value(
        QStringLiteral("animationsEnabled"), true).toBool());
    settings.setValue(QStringLiteral("Behavior/ShowAvatars"), arguments.value(
        QStringLiteral("showAvatars"), true).toBool());
    settings.setValue(QStringLiteral("Behavior/RememberLastUser"), arguments.value(
        QStringLiteral("rememberLastUser"), true).toBool());
    settings.setValue(QStringLiteral("Indicators/ShowBattery"), arguments.value(
        QStringLiteral("showBattery"), true).toBool());
    settings.setValue(QStringLiteral("Indicators/ShowSystemResources"), arguments.value(
        QStringLiteral("showSystemResources"), true).toBool());
    settings.sync();

    if (settings.status() == QSettings::AccessError)
        return helperError(QStringLiteral("Could not write /etc/qmlgreet/qmlgreet.conf."), 1001);
    if (settings.status() == QSettings::FormatError)
        return helperError(QStringLiteral("/etc/qmlgreet/qmlgreet.conf is not a valid INI file."), 1002);

    KAuth::ActionReply reply = KAuth::ActionReply::SuccessReply();
    reply.setData(readSettings());
    return reply;
}

KAUTH_HELPER_MAIN("org.maui.settings.qmlgreet", QmlGreetHelper)
