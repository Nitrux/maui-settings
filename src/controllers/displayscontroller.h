#pragma once

#include <QHash>
#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QSet>

class DisplaysController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(bool configAvailable READ configAvailable NOTIFY settingsChanged)
    Q_PROPERTY(bool automatic READ automatic WRITE setAutomatic NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList monitors READ monitors NOTIFY monitorsChanged)
    Q_PROPERTY(int selectedMonitorIndex READ selectedMonitorIndex WRITE setSelectedMonitorIndex NOTIFY selectedMonitorChanged)
    Q_PROPERTY(QStringList resolutions READ resolutions NOTIFY selectedMonitorChanged)
    Q_PROPERTY(QStringList refreshRates READ refreshRates NOTIFY selectedMonitorChanged)
    Q_PROPERTY(QString selectedResolution READ selectedResolution WRITE setSelectedResolution NOTIFY selectedSettingsChanged)
    Q_PROPERTY(double selectedRefreshRate READ selectedRefreshRate WRITE setSelectedRefreshRate NOTIFY selectedSettingsChanged)
    Q_PROPERTY(int scalePercent READ scalePercent WRITE setScalePercent NOTIFY selectedSettingsChanged)
    Q_PROPERTY(int positionX READ positionX WRITE setPositionX NOTIFY selectedSettingsChanged)
    Q_PROPERTY(int positionY READ positionY WRITE setPositionY NOTIFY selectedSettingsChanged)
    Q_PROPERTY(QVariantList unusedEntries READ unusedEntries NOTIFY settingsChanged)
    Q_PROPERTY(bool hasUnusedEntries READ hasUnusedEntries NOTIFY settingsChanged)

public:
    explicit DisplaysController(QObject *parent = nullptr);

    bool available() const;
    bool configAvailable() const;
    bool automatic() const;
    QVariantList monitors() const;
    int selectedMonitorIndex() const;
    void setSelectedMonitorIndex(int index);
    QStringList resolutions() const;
    QStringList refreshRates() const;
    QString selectedResolution() const;
    void setSelectedResolution(const QString &value);
    double selectedRefreshRate() const;
    void setSelectedRefreshRate(double value);
    int scalePercent() const;
    void setScalePercent(int value);
    int positionX() const;
    void setPositionX(int value);
    int positionY() const;
    void setPositionY(int value);
    QVariantList unusedEntries() const;
    bool hasUnusedEntries() const;
    void setAutomatic(bool value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE void selectMonitor(int index);
    Q_INVOKABLE bool save();
    Q_INVOKABLE void removeUnusedEntry(const QString &output);

Q_SIGNALS:
    void settingsChanged();
    void monitorsChanged();
    void selectedMonitorChanged();
    void selectedSettingsChanged();

private:
    struct Mode { QString value; QString resolution; double refreshRate = 0.0; };
    struct Monitor { QString output; QString description; QList<Mode> modes; QString currentMode; double scale = 1.0; int x = 0; int y = 0; int width = 0; int height = 0; };
    struct ManualSettings { QString mode; double scale = 1.0; int x = 0; int y = 0; };

    void loadConfiguration();
    void loadMonitors();
    void loadConfiguredEntries();
    QString currentMode() const;
    QStringList resolutionsFor(const Monitor &monitor) const;
    QStringList refreshRatesFor(const Monitor &monitor, const QString &resolution) const;
    QString modeFor(const Monitor &monitor, const QString &resolution, double refreshRate) const;
    ManualSettings *selectedSettings();
    const ManualSettings *selectedSettings() const;
    const Monitor *selectedMonitor() const;
    bool writeHyprscreendConfiguration() const;
    bool updateMonitorBlock(const QString &output, const ManualSettings &settings) const;
    bool removeMonitorBlock(const QString &output) const;
    static QString assignmentValue(const QString &line, const QString &key);
    static QString unquote(const QString &value);
    static QString modeResolution(const QString &mode);
    static double modeRefreshRate(const QString &mode);

    QString m_configPath;
    QString m_hyprConfigPath;
    QString m_internalOutput;
    bool m_available = false;
    bool m_configAvailable = false;
    bool m_automatic = true;
    QVariantList m_monitorsVariant;
    QList<Monitor> m_monitors;
    QHash<QString, ManualSettings> m_manualSettings;
    QVariantList m_unusedEntries;
    QSet<QString> m_removedOutputs;
    int m_selectedMonitorIndex = 0;
};
