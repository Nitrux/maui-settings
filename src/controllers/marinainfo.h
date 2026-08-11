#pragma once

#include <QObject>
#include <QString>

class MarinaInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(int iconSize READ iconSize WRITE setIconSize NOTIFY settingsChanged)
    Q_PROPERTY(int edgeMargin READ edgeMargin WRITE setEdgeMargin NOTIFY settingsChanged)
    Q_PROPERTY(QString screenPlacement READ screenPlacement WRITE setScreenPlacement NOTIFY settingsChanged)
    Q_PROPERTY(int dockWidth READ dockWidth WRITE setDockWidth NOTIFY settingsChanged)
    Q_PROPERTY(int dockHeight READ dockHeight WRITE setDockHeight NOTIFY settingsChanged)
    Q_PROPERTY(bool showAboveFullscreen READ showAboveFullscreen WRITE setShowAboveFullscreen NOTIFY settingsChanged)
    Q_PROPERTY(bool autoHide READ autoHide WRITE setAutoHide NOTIFY settingsChanged)
    Q_PROPERTY(int autoHideDelay READ autoHideDelay WRITE setAutoHideDelay NOTIFY settingsChanged)

public:
    explicit MarinaInfo(QObject *parent = nullptr);

    QString configPath() const;
    bool available() const;
    int iconSize() const;
    int edgeMargin() const;
    QString screenPlacement() const;
    int dockWidth() const;
    int dockHeight() const;
    bool showAboveFullscreen() const;
    bool autoHide() const;
    int autoHideDelay() const;

    void setIconSize(int value);
    void setEdgeMargin(int value);
    void setScreenPlacement(const QString &value);
    void setDockWidth(int value);
    void setDockHeight(int value);
    void setShowAboveFullscreen(bool value);
    void setAutoHide(bool value);
    void setAutoHideDelay(int value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    void setChanged();

    QString m_configPath;
    bool m_available = false;
    int m_iconSize = 48;
    int m_edgeMargin = 8;
    QString m_screenPlacement = QStringLiteral("all");
    int m_dockWidth = 0;
    int m_dockHeight = 0;
    bool m_showAboveFullscreen = false;
    bool m_autoHide = false;
    int m_autoHideDelay = 650;
};
