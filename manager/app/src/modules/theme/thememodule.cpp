#include "thememodule.h"
#include "theme.h"

#include <QVariant>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QFontDatabase>

#include <KLocalizedString>

#include <map>

static QString gtkRc2Path()
{
    return QDir::homePath() + QLatin1String("/.gtkrc-2.0");
}

static QString gtk3SettingsIniPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/gtk-3.0/settings.ini");
}

void ThemeModule::updateGtk3Config()
{
    qDebug() << "Request to update GTK3 configs";

    if(!m_manager)
        return;

    QSettings settings(gtk3SettingsIniPath(), QSettings::IniFormat);
    settings.clear();
    //    settings.setIniCodec("UTF-8");
    settings.beginGroup("Settings");

    QFont font;
    font.fromString(m_manager->defaultFont());
    // font
    settings.setValue("gtk-font-name", QString(QStringLiteral("%1 %2")).arg(font.family()).arg(QString::number(font.pointSize())));
    // dark mode
    settings.setValue("gtk-application-prefer-dark-theme", m_manager->styleType() == 1);
    // icon theme
    settings.setValue("gtk-icon-theme-name", m_manager->iconTheme());
    // other
    settings.setValue("gtk-enable-animations", true);
    // theme
    settings.setValue("gtk-theme-name", m_manager->styleType() == 1 ? QStringLiteral("Adwaita-dark") : QStringLiteral("Adwaita-light"));
    settings.sync();
}

ThemeModule::ThemeModule(QObject *parent) :AbstractModule(QStringLiteral("theme"),
                                                          i18n("Theme"),
                                                          i18n("Apperance"),
                                                          QStringLiteral("qrc:/MauiSettingsModule/src/modules/theme/ThemeModuleView.qml"),
                                                          QStringLiteral("preferences-desktop-theme"),
                                                          i18n("Accent colors, icon sets, adaptive colorscheme."),

                                                          QStringList{i18n("look"),
                                                          i18n("theme"),
                                                          i18n("color"),
                                                          i18n("icons"),
                                                          i18n("dark mode"),
                                                          i18n("fonts"),
                                                          i18n("dark"),
                                                          i18n("adaptive"),
                                                          i18n("color scheme"),
                                                          i18n("csd"),
                                                          i18n("window controls"),
                                                          i18n("style")},

                                                          parent)
  , m_manager(nullptr)
{
    qRegisterMetaType<Theme *>("const Theme*"); // this is needed for QML to know of Theme
}

Theme *ThemeModule::manager()
{
    if(!m_manager)
    {
        m_manager = new Theme(this);
        connect(m_manager, &MauiMan::ThemeManager::iconThemeChanged, [this](QString)
        {
            qDebug() << "Request to update IconTheme";

            updateGtk3Config();
        });
    }

    return m_manager;
}

QFont ThemeModule::getFont(const QString &desc)
{
    QFont font;
    font.fromString(desc);
    
    return font;
}

QString ThemeModule::fontToString(const QFont &font)
{
    return font.toString();
}

QStringList ThemeModule::fontStyles(const QFont &font)
{
    QFontDatabase db;
    return db.styles(font.family());
}

QStringList ThemeModule::fontPointSizes(const QFont &font)
{
    QFontDatabase db;
    QStringList res;

    for(auto value : db.smoothSizes(font.family(), font.styleName()))
    {
        res << QString::number(value);
    }

    return res;
}

