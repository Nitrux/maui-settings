/*
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2000 Antonio Larrosa <larrosa@kde.org>
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@privat.broulik.de>

    KDE Frameworks 5 port
    SPDX-FileCopyrightText: 2013 Jonathan Riddell <jr@jriddell.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "iconsmodel.h"

#include <QCollator>
#include <QFileIconProvider>

#include <KIconTheme>

#include <QUrl>
#include <QDebug>

IconsModel::IconsModel( QObject *parent)
    : QAbstractListModel(parent)
{
    load();
}

IconsModel::~IconsModel() = default;

int IconsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_data.count();
}

QVariant IconsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.count()) {
        return QVariant();
    }

    const auto &item = m_data.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        return item.display;
    case ThemeNameRole:
        return item.themeName;
    case DescriptionRole:
        return item.description;
    case RemovableRole:
        return item.removable;
    case PendingDeletionRole:
        return item.pendingDeletion;
    case ScreenshotRole:
        return item.screenshot;
    case IconsRole:
    {
        if(item.icons.isEmpty())
        {
            item.icons = previewIcons(item.themeName);
        }
        return item.icons;
    }
    }

    return QVariant();
}


QHash<int, QByteArray> IconsModel::roleNames() const
{
    return QHash<int, QByteArray>  {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {DescriptionRole, QByteArrayLiteral("description")},
        {ThemeNameRole, QByteArrayLiteral("themeName")},
        {RemovableRole, QByteArrayLiteral("removable")},
        {PendingDeletionRole, QByteArrayLiteral("pendingDeletion")},
        {IconsRole, QByteArrayLiteral("icons")},
        {ScreenshotRole, QByteArrayLiteral("screenshot")}};
}

void IconsModel::load()
{
    beginResetModel();

    m_data.clear();

    const QStringList themes = KIconTheme::list();

    m_data.reserve(themes.count());

    for (const QString &themeName : themes)
    {
        KIconTheme theme(themeName);
        if (!theme.isValid())
        {
            qWarning() << "Not a valid theme" << themeName;
        }

        if (theme.isHidden())
            continue;

        IconsModelData item
        {
            theme.name(),
                    themeName,
                    theme.description(),
                    theme.screenshot(),
                    QStringList{},
            themeName != KIconTheme::defaultThemeName() && QFileInfo(theme.dir()).isWritable(),
                    false // pending deletion
        };

        m_data.append(item);
    }

    // Sort case-insensitively
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(m_data.begin(), m_data.end(), [&collator](const IconsModelData &a, const IconsModelData &b) {
        return collator.compare(a.display, b.display) < 0;
    });

    endResetModel();
}

QStringList IconsModel::previewIcons(const QString &name) const
{
    KIconTheme theme(name);
    const auto lookupIcons = QStringList{"folder", "application-text", "preferences-mail", "preferences-color", "preferences-desktop-emoticons", "office-calendar", "audio-speakers", "computer", "joystick", "network-wired",  "weather-clear", "image-png", "vvave",  "application-x-project"};

    QStringList icons;

    uint i = 0;
    for(const auto &iconName : lookupIcons)
    {
        auto url  = QUrl::fromLocalFile(theme.iconPathByName(iconName, 48, KIconLoader::MatchBest, 1));

        if(!url.isEmpty())
        {
            icons << url.toString();
            i++;
        }

        if(i == 3)
            break;
    }

    return icons;
}

IconsProxyModel::IconsProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void IconsProxyModel::setFilter(QString filter)
{
    if (this->m_filter == filter)
        return;

    this->m_filter = filter;
    this->setFilterFixedString(this->m_filter);
    Q_EMIT this->filterChanged(this->m_filter);
}

QString IconsProxyModel::filter() const
{
    return m_filter;
}

void IconsProxyModel::resetFilter()
{
    this->setFilterRegExp("");
    this->invalidateFilter();
}

bool IconsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (this->filterRole() != Qt::DisplayRole)
    {
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto data = this->sourceModel()->data(index, this->filterRole()).toString();
        return data.contains(this->filterRegExp());
    }

    const auto roleNames = this->sourceModel()->roleNames();
    for (const auto &role : roleNames.keys()) {
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        const auto data = this->sourceModel()->data(index, role).toString();
        if (data.contains(this->filterRegExp()))
            return true;
        else
            continue;
    }

    return false;
}

bool IconsProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    return true;
}
