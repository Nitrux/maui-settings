#pragma once

#include <QObject>
#include <QString>

class BackgroundInfo;
class KdeGlobalsInfo;
class QFileSystemWatcher;
class QTimer;

namespace MauiMan
{
class ThemeManager;
}

class WallpaperColorsController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool kdeSynchronizationEnabled READ kdeSynchronizationEnabled WRITE setKdeSynchronizationEnabled NOTIFY kdeSynchronizationEnabledChanged)

public:
    explicit WallpaperColorsController(MauiMan::ThemeManager *theme,
                                       BackgroundInfo *background,
                                       KdeGlobalsInfo *kde,
                                       QObject *parent = nullptr);

    bool kdeSynchronizationEnabled() const;
    void setKdeSynchronizationEnabled(bool enabled);
    Q_INVOKABLE void synchronize();

Q_SIGNALS:
    void kdeSynchronizationEnabledChanged();

private:
    void publishWallpaperSource(const QString &path);
    void refreshWallpaperSource();
    void watchSourceFile(const QString &path);
    void clearSourceWatcher();
    void onThemeSourceChanged(const QString &source);
    void synchronizeKde(const QString &source);
    bool writeGeneratedScheme(const QString &source);
    void restorePreviousScheme();
    void persistSettings() const;
    QString generatedSchemePath() const;
    static QString canonicalImagePath(const QString &path);

    MauiMan::ThemeManager *m_theme;
    BackgroundInfo *m_background;
    KdeGlobalsInfo *m_kde;
    bool m_kdeSynchronizationEnabled = false;
    QString m_previousKdeScheme;
    bool m_hasPreviousKdeScheme = false;
    QFileSystemWatcher *m_sourceWatcher = nullptr;
    QTimer *m_sourceSyncTimer = nullptr;
    QString m_watchedSourcePath;
    QString m_currentSource;
};
