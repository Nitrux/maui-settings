#include "thememodule.h"
#include "theme.h"
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QFontDatabase>

#include <KI18n/KLocalizedString>
#include <KSharedConfig>
#include <KColorScheme>
#include <KConfigGroup>

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
    settings.setIniCodec("UTF-8");
    settings.beginGroup("Settings");

    QFont font;
    font.fromString(m_manager->defaultFont());
    // font
    settings.setValue("gtk-font-name", QString("%1 %2").arg(font.family()).arg(QString::number(font.pointSize())));
    // dark mode
    settings.setValue("gtk-application-prefer-dark-theme", m_manager->styleType() == 1);
    // icon theme
    settings.setValue("gtk-icon-theme-name", m_manager->iconTheme());
    // other
    settings.setValue("gtk-enable-animations", true);
    // theme
    settings.setValue("gtk-theme-name", m_manager->styleType() == 1 ? "Adwaita-dark" : "Adwaita-light");
    settings.sync();
}

ThemeModule::ThemeModule(QObject *parent) :AbstractModule("theme",
                                                          i18n("Theme"),
                                                          i18n("Apperance"),
                                                          "qrc:/modules/theme/ThemeModuleView.qml",
                                                          "preferences-desktop-theme",
                                                          i18n("Accent colors, icon sets, adaptive colorscheme."),
{"look", "theme", "color", "icons", "dark mode", "dark", "adaptive"},
                                                          parent)
  , m_manager(nullptr)
  , m_colorSchemeModel(nullptr)
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

ColorSchemesModel *ThemeModule::colorSchemeModel()
{
    if(!m_colorSchemeModel)
    {
        m_colorSchemeModel = new ColorSchemesModel(this);
    }

    return m_colorSchemeModel;
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

QStringList ColorSchemesModel::schemeColors(const QString &scheme) const
{
    KSharedConfigPtr schemeConfig = KSharedConfig::openConfig(scheme, KConfig::SimpleConfig);

    KColorScheme activeWindow(QPalette::Active, KColorScheme::Window, schemeConfig);
    KColorScheme activeButton(QPalette::Active, KColorScheme::Button, schemeConfig);
    KColorScheme activeView(QPalette::Active, KColorScheme::View, schemeConfig);
    KColorScheme activeSelection(QPalette::Active, KColorScheme::Selection, schemeConfig);

    return QStringList{activeWindow.background().color().name(), activeButton.background().color().name(), activeView.background().color().name(), activeSelection.background().color().name(), activeWindow.foreground().color().name()};

}



ColorSchemesModel::ColorSchemesModel(QObject *parent) : QAbstractListModel(parent)
{
    beginResetModel();
    m_data.clear();

#ifndef Q_OS_ANDROID
    // Fill the model with all *.colors files from the XDG_DATA_DIRS, sorted by "Name".
    // If two color schemes, in user's $HOME and e.g. /usr, respectively, have the same
    // name, the one under $HOME overrides the other one
    const QStringList dirPaths =
            QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes"), QStandardPaths::LocateDirectory);
#else
    const QStringList dirPaths{QStringLiteral("assets:/share/color-schemes")};
#endif

    std::map<QString, QString> map;
    for (const QString &dirPath : dirPaths)
    {
        const QDir dir(dirPath);
        const QStringList fileNames = dir.entryList({QStringLiteral("*.colors")});
        for (const auto &file : fileNames)
        {
            map.insert({file, dir.filePath(file)});
        }
    }

    for (const auto &[key, schemeFilePath] : map)
    {
        KSharedConfigPtr config = KSharedConfig::openConfig(schemeFilePath, KConfig::SimpleConfig);
        KConfigGroup group(config, QStringLiteral("General"));
        const QString name = group.readEntry("Name", QFileInfo(schemeFilePath).baseName());
        const QString id = key.chopped(QLatin1String(".colors").size()); // Remove .colors ending
        const KColorSchemeModelData data = {id, name, schemeFilePath, QStringList()};
        m_data.append(data);
    }

    m_data.insert(0, {QStringLiteral("Default"), i18n("Default"), QString(), QStringList()});
    endResetModel();

}

int ColorSchemesModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        return 0;
    }

    return m_data.count();
}

QVariant ColorSchemesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.row() >= m_data.count()))
    {
        return QVariant();
    }

    switch (role)
    {
    case NameRole:
        return m_data.at(index.row()).name;
    case PreviewRole: {
        auto &item = m_data[index.row()];
        if (item.preview.isEmpty()) {
            item.preview = schemeColors(item.path);
        }
        return item.preview;
    }
    case PathRole:
        return m_data.at(index.row()).path;
    case FileRole:
        return m_data.at(index.row()).id;
    default:
        return QVariant();
    }


}

QHash<int, QByteArray> ColorSchemesModel::roleNames() const
{
    return QHash<int, QByteArray>{{Roles::NameRole, "name"}, {Roles::PreviewRole, "preview"}, {Roles::PathRole, "path"}, {Roles::FileRole, "file"}};
}
