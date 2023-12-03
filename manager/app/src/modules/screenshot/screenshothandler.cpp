#include "screenshothandler.h"
#include <KDesktopFile>

#include <QSettings>
#include <QDebug>

static QVariantMap desktopData(const QString &uri)
{
    KDesktopFile desktop(uri+QStringLiteral(".desktop"));
    return QVariantMap {{QStringLiteral("id"), uri},
        {QStringLiteral("name"), desktop.readName()},
        {QStringLiteral("icon"), desktop.readIcon()},
        {QStringLiteral("comment"), desktop.readComment()}};
}

ScreenshotHandler::ScreenshotHandler(QObject *parent) : CaskScreenshot(parent)
  ,m_settings(new QSettings(this))
{
    // connect(this, &ScreenshotHandler::blacklistedModelChanged, [this](QStringList)
    // {
    //     setBlacklistedModel();
    // });

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

