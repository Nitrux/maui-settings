#include "theme.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDirIterator>
#include <QSettings>

Theme::Theme(QObject *parent) : MauiMan::ThemeManager(parent)
  ,m_windowDecorationsModel(new WindowDecorationsModel(this))
{
    qDebug( " INIT THEME MANAGER");
    qRegisterMetaType<WindowDecorationsModel *>("const WindowDecorationsModel*"); // this is needed for QML to know of FMList in the search method

}

WindowDecorationsModel *Theme::windowDecorationsModel() const
{
    return m_windowDecorationsModel;
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
        auto confFile = currentDir.absolutePath() +"/config.conf";
        qDebug() << "WINDOCSDSTUF" << confFile;

        if(!QFileInfo(confFile).exists())
        {
            continue;
        }

        auto name = currentDir.dirName();
        QSettings conf (confFile, QSettings::IniFormat);
        conf.beginGroup ("Decoration");
        auto source = QUrl::fromLocalFile(currentDir.absolutePath()).toString()+"/"+ conf.value("Source", "undefined.qml").toString();
        conf.endGroup ();

        if(!QFileInfo(QUrl(source).toLocalFile()).exists())
        {
            continue;
        }

        m_decos << Decoration{name, source};
    }
}
