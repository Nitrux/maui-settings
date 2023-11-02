#include "theme.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDirIterator>
#include <QSettings>

#include <KSharedConfig>
#include <KColorScheme>
#include <KConfigGroup>

#include <KLocalizedString>

#include <MauiMan4/backgroundmanager.h>

#include "iconsmodel.h"

Theme::Theme(QObject *parent) : MauiMan::ThemeManager(parent)
  ,m_windowDecorationsModel(nullptr)
  ,m_iconsModel(nullptr)
  ,m_colorSchemeModel(nullptr)
{
    qDebug( " INIT THEME MANAGER");
    qRegisterMetaType<WindowDecorationsModel *>("const WindowDecorationsModel*"); // this is needed for QML to know of WindowDecorations
    qRegisterMetaType<IconsProxyModel *>("const IconsProxyModel*"); // this is needed for QML to know of WindowDecorations
    qRegisterMetaType<ColorSchemesModelProxy *>("const ColorSchemesModelProxy*"); // this is needed for QML to know of WindowDecorations

    auto bg = new  MauiMan::BackgroundManager(this);
    connect(bg, &MauiMan::BackgroundManager::wallpaperSourceChanged, [this](const QString &source)
    {
        m_wallpaper = source;
        Q_EMIT wallpaperChanged(m_wallpaper);
    });
    m_wallpaper = bg->wallpaperSource();
}

WindowDecorationsModel *Theme::windowDecorationsModel()
{
    if(!m_windowDecorationsModel)
    {
        m_windowDecorationsModel = new WindowDecorationsModel(this);
    }

    return m_windowDecorationsModel;
}

IconsProxyModel *Theme::iconsModel()
{
    if(!m_iconsModel)
    {
        auto model = new IconsModel(this);
        m_iconsModel = new IconsProxyModel(this);
        m_iconsModel->setSourceModel(model);
    }
    return m_iconsModel;
}

WindowDecorationsModel::WindowDecorationsModel(Theme *parent) : QAbstractListModel(parent)
{
    this->getDecorations();
}

int WindowDecorationsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_decos.count();
}

QVariant WindowDecorationsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    auto deco = m_decos.at(index.row());
    switch(role)
    {
    case Name: return QVariant(deco.name);
    case Source: return QVariant(deco.source);
    default: return QVariant();
    }
}

QHash<int, QByteArray> WindowDecorationsModel::roleNames() const
{
    return {{Roles::Name, "name"}, {Roles::Source, "source"}};
}

void WindowDecorationsModel::getDecorations()
{
    auto controlsDir = QStandardPaths::locate (QStandardPaths::GenericDataLocation, QStringLiteral("org.mauikit.controls/csd"), QStandardPaths::LocateDirectory);

    qDebug() << " Trying to find WINDOCSDSTUF" << controlsDir;

    QDir dir(controlsDir);
    if(!dir.exists())
    {
        return;
    }

    QDirIterator it(controlsDir, QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext())
    {
        QDir currentDir(it.next());
        QString confFile = currentDir.absolutePath() + QStringLiteral("/config.conf");
        qDebug() << "WINDOCSDSTUF" << confFile;

        if(!QFileInfo(confFile).exists())
        {
            continue;
        }

        auto name = currentDir.dirName();
        QSettings conf (confFile, QSettings::IniFormat);
        conf.beginGroup ("Decoration");
        QString source = QUrl::fromLocalFile(currentDir.absolutePath()).toString()+ QStringLiteral("/") + conf.value("Source", QStringLiteral("undefined.qml")).toString();
        conf.endGroup ();

        if(!QFileInfo(QUrl(source).toLocalFile()).exists())
        {
            continue;
        }

        m_decos << Decoration{name, source};
    }
}

QStringList ColorSchemesModel::schemeColors(const QString &scheme) const
{
    KSharedConfigPtr schemeConfig = KSharedConfig::openConfig(scheme, KConfig::SimpleConfig);

    KColorScheme activeWindow(QPalette::Active, KColorScheme::Window, schemeConfig);
    KColorScheme activeButton(QPalette::Active, KColorScheme::Button, schemeConfig);
    KColorScheme activeView(QPalette::Active, KColorScheme::View, schemeConfig);
    KColorScheme activeSelection(QPalette::Active, KColorScheme::Selection, schemeConfig);

    return {activeWindow.background().color().name(),
                activeWindow.foreground().color().name(),
                activeButton.background().color().name(),
                activeView.background().color().name(),
                activeSelection.background().color().name()};

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
        const QString name = group.readEntry(QStringLiteral("Name"), QFileInfo(schemeFilePath).baseName());
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

ColorSchemesModelProxy *Theme::colorSchemeModel()
{
    if(!m_colorSchemeModel)
    {
        auto model = new ColorSchemesModel(this);
        m_colorSchemeModel = new ColorSchemesModelProxy(this);
        m_colorSchemeModel->setSourceModel(model);
    }

    return m_colorSchemeModel;
}

QString Theme::wallpaper() const
{
    return m_wallpaper;
}

ColorSchemesModelProxy::ColorSchemesModelProxy(QObject *parent) : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void ColorSchemesModelProxy::setFilter(QString filter)
{
    if (this->m_filter == filter)
        return;

    this->m_filter = filter;
    this->setFilterFixedString(this->m_filter);
    Q_EMIT this->filterChanged(this->m_filter);
}

QString ColorSchemesModelProxy::filter() const
{
    return m_filter;
}

void ColorSchemesModelProxy::resetFilter()
{
    this->setFilterRegularExpression(QStringLiteral(""));
    this->invalidateFilter();
}

bool ColorSchemesModelProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (this->filterRole() != Qt::DisplayRole)
    {
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto data = this->sourceModel()->data(index, this->filterRole()).toString();
        return data.contains(this->filterRegularExpression());
    }

    const auto roleNames = this->sourceModel()->roleNames();
    for (const auto &role : roleNames.keys()) {
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto data = this->sourceModel()->data(index, role).toString();
        if (data.contains(this->filterRegularExpression()))
            return true;
        else
            continue;
    }

    return false;
}

bool ColorSchemesModelProxy::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    return true;
}
