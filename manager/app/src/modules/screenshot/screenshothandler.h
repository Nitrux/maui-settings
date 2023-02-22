#pragma once

#include <CaskServer/caskscreenshot.h>
#include <QObject>
#include <QVariantList>
#include <QVector>
#include <QPair>

class QSettings;
class ScreenshotHandler : public CaskScreenshot
{    
    Q_OBJECT
    Q_PROPERTY(QVariantList blacklistedModel READ blacklistedModel NOTIFY blacklistedModelChanged)

    QVariantList m_blacklistedModel;

public:
    ScreenshotHandler(QObject *parent = nullptr);
    QVariantList blacklistedModel() const;

private:
    QSettings *m_settings;
    void setBlacklistedModel();

signals:
    void blacklistedModelChanged(QVariantList blacklisted);
};

