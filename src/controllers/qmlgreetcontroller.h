#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class QmlGreetController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(bool saveAvailable READ saveAvailable NOTIFY saveAvailableChanged)
    Q_PROPERTY(QString wallpaperDirectory READ wallpaperDirectory NOTIFY wallpaperPathChanged)
    Q_PROPERTY(QString wallpaperPath READ wallpaperPath WRITE setWallpaperPath NOTIFY wallpaperPathChanged)
    Q_PROPERTY(QString colorSchemeDirectory READ colorSchemeDirectory NOTIFY colorSchemePathChanged)
    Q_PROPERTY(QString colorSchemePath READ colorSchemePath WRITE setColorSchemePath NOTIFY colorSchemePathChanged)
    Q_PROPERTY(QString iconTheme READ iconTheme WRITE setIconTheme NOTIFY iconThemeChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)
    Q_PROPERTY(QString avatarDirectory READ avatarDirectory NOTIFY avatarPathChanged)
    Q_PROPERTY(QString avatarPath READ avatarPath WRITE setAvatarPath NOTIFY avatarPathChanged)
    Q_PROPERTY(QString timeFormat READ timeFormat WRITE setTimeFormat NOTIFY timeFormatChanged)
    Q_PROPERTY(QString dateFormat READ dateFormat WRITE setDateFormat NOTIFY dateFormatChanged)
    Q_PROPERTY(bool blurEnabled READ blurEnabled WRITE setBlurEnabled NOTIFY blurEnabledChanged)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled WRITE setAnimationsEnabled NOTIFY animationsEnabledChanged)
    Q_PROPERTY(bool overlayEnabled READ overlayEnabled WRITE setOverlayEnabled NOTIFY overlayEnabledChanged)
    Q_PROPERTY(double overlayOpacity READ overlayOpacity WRITE setOverlayOpacity NOTIFY overlayOpacityChanged)
    Q_PROPERTY(QStringList availableSessions READ availableSessions NOTIFY availableSessionsChanged)
    Q_PROPERTY(QString defaultSession READ defaultSession WRITE setDefaultSession NOTIFY defaultSessionChanged)
    Q_PROPERTY(bool showAvatars READ showAvatars WRITE setShowAvatars NOTIFY showAvatarsChanged)
    Q_PROPERTY(bool rememberLastUser READ rememberLastUser WRITE setRememberLastUser NOTIFY rememberLastUserChanged)
    Q_PROPERTY(bool showBattery READ showBattery WRITE setShowBattery NOTIFY showBatteryChanged)
    Q_PROPERTY(bool showSystemResources READ showSystemResources WRITE setShowSystemResources NOTIFY showSystemResourcesChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(bool saving READ saving NOTIFY savingChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit QmlGreetController(QObject *parent = nullptr);

    QString configPath() const;
    bool saveAvailable() const;
    QString wallpaperDirectory() const;
    QString wallpaperPath() const;
    QString colorSchemeDirectory() const;
    QString colorSchemePath() const;
    QString iconTheme() const;
    QString fontFamily() const;
    int fontSize() const;
    int borderRadius() const;
    QString avatarDirectory() const;
    QString avatarPath() const;
    QString timeFormat() const;
    QString dateFormat() const;
    bool blurEnabled() const;
    bool animationsEnabled() const;
    bool overlayEnabled() const;
    double overlayOpacity() const;
    QStringList availableSessions() const;
    QString defaultSession() const;
    bool showAvatars() const;
    bool rememberLastUser() const;
    bool showBattery() const;
    bool showSystemResources() const;
    bool dirty() const;
    bool loading() const;
    bool saving() const;
    QString statusMessage() const;
    QString errorMessage() const;

    void setWallpaperPath(const QString &value);
    void setColorSchemePath(const QString &value);
    void setIconTheme(const QString &value);
    void setFontFamily(const QString &value);
    void setFontSize(int value);
    void setBorderRadius(int value);
    void setAvatarPath(const QString &value);
    void setTimeFormat(const QString &value);
    void setDateFormat(const QString &value);
    void setBlurEnabled(bool value);
    void setAnimationsEnabled(bool value);
    void setOverlayEnabled(bool value);
    void setOverlayOpacity(double value);
    void setDefaultSession(const QString &value);
    void setShowAvatars(bool value);
    void setRememberLastUser(bool value);
    void setShowBattery(bool value);
    void setShowSystemResources(bool value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE void refreshSessions();
    Q_INVOKABLE bool save();

Q_SIGNALS:
    void saveAvailableChanged();
    void wallpaperPathChanged();
    void colorSchemePathChanged();
    void iconThemeChanged();
    void fontFamilyChanged();
    void fontSizeChanged();
    void borderRadiusChanged();
    void avatarPathChanged();
    void timeFormatChanged();
    void dateFormatChanged();
    void blurEnabledChanged();
    void animationsEnabledChanged();
    void overlayEnabledChanged();
    void overlayOpacityChanged();
    void availableSessionsChanged();
    void defaultSessionChanged();
    void showAvatarsChanged();
    void rememberLastUserChanged();
    void showBatteryChanged();
    void showSystemResourcesChanged();
    void dirtyChanged();
    void loadingChanged();
    void savingChanged();
    void statusMessageChanged();
    void errorMessageChanged();
    void configurationChanged(const QString &key);
    void saveSucceeded();
    void saveFailed(const QString &message, bool authenticationCancelled);
    void reloadFailed(const QString &message);

private:
    QVariantMap stagedValues() const;
    QVariantMap changedValues() const;
    void applyValues(const QVariantMap &values);
    void updateDirty();
    void refreshSaveAvailability();
    void setLoading(bool value);
    void setSaving(bool value);
    void setStatusMessage(const QString &value);
    void setErrorMessage(const QString &value);
    static QString normalizeLocalPath(const QString &value);
    static QString authorizationError(int errorCode, const QString &fallback);

    bool m_saveAvailable = false;
    QString m_wallpaperPath;
    QString m_colorSchemePath;
    QString m_iconTheme = QStringLiteral("hicolor");
    QString m_fontFamily = QStringLiteral("Noto Sans");
    int m_fontSize = 10;
    int m_borderRadius = 8;
    QString m_avatarPath;
    QString m_timeFormat = QStringLiteral("hh:mm");
    QString m_dateFormat = QStringLiteral("dddd, dd MMMM yyyy");
    bool m_blurEnabled = true;
    bool m_animationsEnabled = true;
    bool m_overlayEnabled = true;
    double m_overlayOpacity = 0.76;
    QStringList m_availableSessions;
    QString m_defaultSession;
    bool m_showAvatars = true;
    bool m_rememberLastUser = true;
    bool m_showBattery = true;
    bool m_showSystemResources = true;
    QVariantMap m_savedValues;
    bool m_dirty = false;
    bool m_loading = false;
    bool m_saving = false;
    QString m_statusMessage;
    QString m_errorMessage;
};
