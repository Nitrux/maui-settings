#pragma once

#include <QObject>
#include <QString>

class BackgroundInfo;
class KdeGlobalsInfo;
class HyprlandInfo;
class QFileSystemWatcher;
class QTimer;

namespace MauiKit
{
class AdaptivePalette;
}

namespace MauiMan
{
class ThemeManager;
}

class WallpaperColorsController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool kdeSynchronizationEnabled READ kdeSynchronizationEnabled WRITE setKdeSynchronizationEnabled NOTIFY kdeSynchronizationEnabledChanged)
    Q_PROPERTY(bool vicinaeAvailable READ vicinaeAvailable CONSTANT)
    Q_PROPERTY(bool vicinaeSynchronizationEnabled READ vicinaeSynchronizationEnabled WRITE setVicinaeSynchronizationEnabled NOTIFY vicinaeSynchronizationEnabledChanged)

public:
    explicit WallpaperColorsController(MauiMan::ThemeManager *theme,
                                       BackgroundInfo *background,
                                       KdeGlobalsInfo *kde,
                                       HyprlandInfo *hyprland,
                                       QObject *parent = nullptr);

    bool kdeSynchronizationEnabled() const;
    void setKdeSynchronizationEnabled(bool enabled);
    bool vicinaeAvailable() const;
    bool vicinaeSynchronizationEnabled() const;
    void setVicinaeSynchronizationEnabled(bool enabled);
    Q_INVOKABLE void synchronize();

Q_SIGNALS:
    void kdeSynchronizationEnabledChanged();
    void vicinaeSynchronizationEnabledChanged();

private:
    void publishWallpaperSource(const QString &path);
    void synchronizeWallpaperSource(const QString &path);
    void updateWallpaperSource(const QString &path, bool synchronizeGreeter);
    void refreshWallpaperSource();
    void watchSourceFile(const QString &path);
    void clearSourceWatcher();
    void onThemeSourceChanged(const QString &source);
    void synchronizeKde(const QString &source, bool synchronizeGreeter);
    void synchronizeVicinae(const QString &source);
    void refreshVicinaeTheme(const QString &themeId);
    void synchronizeHyprlandBorders(const MauiKit::AdaptivePalette &palette);
    bool writeGeneratedScheme(const MauiKit::AdaptivePalette &palette);
    bool writeGeneratedVicinaeTheme(const MauiKit::AdaptivePalette &palette,
                                    const QString &themeId,
                                    const QString &variant,
                                    const QString &parentId);
    bool writeVicinaeSettings(bool generatedThemes, bool useLightTheme);
    bool useLightVicinaeTheme(const MauiKit::AdaptivePalette &palette) const;
    void restorePreviousScheme();
    void persistSettings() const;
    QString generatedSchemePath() const;
    QString generatedVicinaeThemePath(const QString &themeId) const;
    QString vicinaeSettingsPath() const;
    static QString canonicalImagePath(const QString &path);

    MauiMan::ThemeManager *m_theme;
    BackgroundInfo *m_background;
    KdeGlobalsInfo *m_kde;
    HyprlandInfo *m_hyprland;
    bool m_kdeSynchronizationEnabled = false;
    bool m_vicinaeAvailable = false;
    bool m_vicinaeSynchronizationEnabled = false;
    QString m_previousKdeScheme;
    bool m_hasPreviousKdeScheme = false;
    QFileSystemWatcher *m_sourceWatcher = nullptr;
    QTimer *m_sourceSyncTimer = nullptr;
    QString m_watchedSourcePath;
    QString m_currentSource;
};
