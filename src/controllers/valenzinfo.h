#pragma once

#include <QObject>
#include <QString>

class ValenzInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(int barHeight READ barHeight WRITE setBarHeight NOTIFY settingsChanged)
    Q_PROPERTY(int barLayerSpacingTop READ barLayerSpacingTop WRITE setBarLayerSpacingTop NOTIFY settingsChanged)
    Q_PROPERTY(int barLayerSpacingBottom READ barLayerSpacingBottom WRITE setBarLayerSpacingBottom NOTIFY settingsChanged)
    Q_PROPERTY(int barLayerSpacingLeft READ barLayerSpacingLeft WRITE setBarLayerSpacingLeft NOTIFY settingsChanged)
    Q_PROPERTY(int barLayerSpacingRight READ barLayerSpacingRight WRITE setBarLayerSpacingRight NOTIFY settingsChanged)
    Q_PROPERTY(QString screenPlacement READ screenPlacement WRITE setScreenPlacement NOTIFY settingsChanged)
    Q_PROPERTY(QString controlCenterIconMode READ controlCenterIconMode WRITE setControlCenterIconMode NOTIFY settingsChanged)
    Q_PROPERTY(QString lightColorScheme READ lightColorScheme WRITE setLightColorScheme NOTIFY settingsChanged)
    Q_PROPERTY(QString darkColorScheme READ darkColorScheme WRITE setDarkColorScheme NOTIFY settingsChanged)
    Q_PROPERTY(QString controlCenterPowerCommand READ controlCenterPowerCommand WRITE setControlCenterPowerCommand NOTIFY settingsChanged)
    Q_PROPERTY(QString controlCenterSettingsCommand READ controlCenterSettingsCommand WRITE setControlCenterSettingsCommand NOTIFY settingsChanged)
    Q_PROPERTY(QString launcherCommand READ launcherCommand WRITE setLauncherCommand NOTIFY settingsChanged)
    Q_PROPERTY(QString clipboardCommand READ clipboardCommand WRITE setClipboardCommand NOTIFY settingsChanged)
    Q_PROPERTY(QString controlCenterDiskUsagePath READ controlCenterDiskUsagePath WRITE setControlCenterDiskUsagePath NOTIFY settingsChanged)
    Q_PROPERTY(bool mprisAlwaysVisible READ mprisAlwaysVisible WRITE setMprisAlwaysVisible NOTIFY settingsChanged)
    Q_PROPERTY(double weatherLatitude READ weatherLatitude WRITE setWeatherLatitude NOTIFY settingsChanged)
    Q_PROPERTY(double weatherLongitude READ weatherLongitude WRITE setWeatherLongitude NOTIFY settingsChanged)
    Q_PROPERTY(QString weatherTemperatureUnit READ weatherTemperatureUnit WRITE setWeatherTemperatureUnit NOTIFY settingsChanged)
    Q_PROPERTY(int weatherRefreshMinutes READ weatherRefreshMinutes WRITE setWeatherRefreshMinutes NOTIFY settingsChanged)

public:
    explicit ValenzInfo(QObject *parent = nullptr);

    QString configPath() const;
    bool available() const;
    int barHeight() const;
    int barLayerSpacingTop() const;
    int barLayerSpacingBottom() const;
    int barLayerSpacingLeft() const;
    int barLayerSpacingRight() const;
    QString screenPlacement() const;
    QString controlCenterIconMode() const;
    QString lightColorScheme() const;
    QString darkColorScheme() const;
    QString controlCenterPowerCommand() const;
    QString controlCenterSettingsCommand() const;
    QString launcherCommand() const;
    QString clipboardCommand() const;
    QString controlCenterDiskUsagePath() const;
    bool mprisAlwaysVisible() const;
    double weatherLatitude() const;
    double weatherLongitude() const;
    QString weatherTemperatureUnit() const;
    int weatherRefreshMinutes() const;

    void setBarHeight(int value);
    void setBarLayerSpacingTop(int value);
    void setBarLayerSpacingBottom(int value);
    void setBarLayerSpacingLeft(int value);
    void setBarLayerSpacingRight(int value);
    void setScreenPlacement(const QString &value);
    void setControlCenterIconMode(const QString &value);
    void setLightColorScheme(const QString &value);
    void setDarkColorScheme(const QString &value);
    void setControlCenterPowerCommand(const QString &value);
    void setControlCenterSettingsCommand(const QString &value);
    void setLauncherCommand(const QString &value);
    void setClipboardCommand(const QString &value);
    void setControlCenterDiskUsagePath(const QString &value);
    void setMprisAlwaysVisible(bool value);
    void setWeatherLatitude(double value);
    void setWeatherLongitude(double value);
    void setWeatherTemperatureUnit(const QString &value);
    void setWeatherRefreshMinutes(int value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    void setChanged();

    QString m_configPath;
    bool m_available = false;
    int m_barHeight = 56;
    int m_barLayerSpacingTop = 0;
    int m_barLayerSpacingBottom = 0;
    int m_barLayerSpacingLeft = 0;
    int m_barLayerSpacingRight = 0;
    QString m_screenPlacement = QStringLiteral("active");
    QString m_controlCenterIconMode = QStringLiteral("system16");
    QString m_lightColorScheme = QStringLiteral("CatppuccinLatteNitrux");
    QString m_darkColorScheme = QStringLiteral("CatppuccinMochaNitrux");
    QString m_controlCenterPowerCommand = QStringLiteral("qmlogout");
    QString m_controlCenterSettingsCommand = QStringLiteral("systemsettings");
    QString m_launcherCommand = QStringLiteral("vicinae toggle");
    QString m_clipboardCommand = QStringLiteral("vicinae vicinae://launch/clipboard/history");
    QString m_controlCenterDiskUsagePath = QStringLiteral("/");
    bool m_mprisAlwaysVisible = false;
    double m_weatherLatitude = 40.7128;
    double m_weatherLongitude = -74.0060;
    QString m_weatherTemperatureUnit = QStringLiteral("celsius");
    int m_weatherRefreshMinutes = 20;
};
