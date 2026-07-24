#pragma once

#include <QObject>
#include <QString>

class NudgeOsdInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(QString iconMode READ iconMode WRITE setIconMode NOTIFY settingsChanged)
    Q_PROPERTY(int osdWidth READ osdWidth WRITE setOsdWidth NOTIFY settingsChanged)
    Q_PROPERTY(int osdHeight READ osdHeight WRITE setOsdHeight NOTIFY settingsChanged)
    Q_PROPERTY(int bottomOffset READ bottomOffset WRITE setBottomOffset NOTIFY settingsChanged)
    Q_PROPERTY(int hideTimeout READ hideTimeout WRITE setHideTimeout NOTIFY settingsChanged)
    Q_PROPERTY(int showAnimationDuration READ showAnimationDuration WRITE setShowAnimationDuration NOTIFY settingsChanged)
    Q_PROPERTY(int hideAnimationDuration READ hideAnimationDuration WRITE setHideAnimationDuration NOTIFY settingsChanged)
    Q_PROPERTY(int volumeStep READ volumeStep WRITE setVolumeStep NOTIFY settingsChanged)
    Q_PROPERTY(int brightnessStep READ brightnessStep WRITE setBrightnessStep NOTIFY settingsChanged)

public:
    explicit NudgeOsdInfo(QObject *parent = nullptr);

    QString configPath() const;
    QString iconMode() const;
    int osdWidth() const;
    int osdHeight() const;
    int bottomOffset() const;
    int hideTimeout() const;
    int showAnimationDuration() const;
    int hideAnimationDuration() const;
    int volumeStep() const;
    int brightnessStep() const;

    void setIconMode(const QString &value);
    void setOsdWidth(int value);
    void setOsdHeight(int value);
    void setBottomOffset(int value);
    void setHideTimeout(int value);
    void setShowAnimationDuration(int value);
    void setHideAnimationDuration(int value);
    void setVolumeStep(int value);
    void setBrightnessStep(int value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    void setChanged();

    QString m_configPath;
    QString m_iconMode = QStringLiteral("system");
    int m_osdWidth = 292;
    int m_osdHeight = 66;
    int m_bottomOffset = 114;
    int m_hideTimeout = 2000;
    int m_showAnimationDuration = 200;
    int m_hideAnimationDuration = 200;
    int m_volumeStep = 5;
    int m_brightnessStep = 10;
};
