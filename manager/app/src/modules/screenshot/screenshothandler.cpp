#include "screenshothandler.h"
#include <KDesktopFile>

#include <QSettings>
#include <QDebug>

static QVariantMap desktopData(const QString &uri)
{
    KDesktopFile desktop(uri+".desktop");
    return QVariantMap{{"id", uri}, {"name", desktop.readName()}, {"icon", desktop.readIcon()}, {"comment", desktop.readComment()}};
}

ScreenshotHandler::ScreenshotHandler(QObject *parent) : CaskScreenshot(parent)
  ,m_settings(new QSettings(this))
{
    connect(this, &ScreenshotHandler::blacklistedChanged, [this](QStringList)
    {
        setBlacklistedModel();
    });

    setBlacklistedModel();
}

QVariantList ScreenshotHandler::blacklistedModel() const
{
    return m_blacklistedModel;
}


void ScreenshotHandler::setBlacklistedModel()
{
    m_blacklistedModel.clear();
    for(const auto &id : blacklisted())
    {
        m_blacklistedModel.append(desktopData(id));
    }

    Q_EMIT blacklistedModelChanged(m_blacklistedModel);
}

