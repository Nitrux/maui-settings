/*
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@privat.broulik.de>

    KDE Frameworks 5 port:
    SPDX-FileCopyrightText: 2013 Jonathan Riddell <jr@jriddell.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include <QString>
#include <QVector>


struct IconsModelData {
    QString display;
    QString themeName;
    QString description;
    QString screenshot;
    mutable QStringList icons;
    bool removable;
    bool pendingDeletion;
};
Q_DECLARE_TYPEINFO(IconsModelData, Q_MOVABLE_TYPE);

class IconsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged RESET resetFilter)

public:
    explicit IconsProxyModel(QObject *parent = nullptr );
    void setFilter(QString filter);

    QString filter() const;
    void resetFilter();

Q_SIGNALS:
    void filterChanged(QString filter);

private:
    QString m_filter;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override final;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override final;
};

class IconsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    IconsModel(QObject *parent = nullptr);
    ~IconsModel() override;

    enum Roles
    {
        ThemeNameRole = Qt::UserRole + 1,
        DescriptionRole,
        RemovableRole,
        PendingDeletionRole,
        ScreenshotRole,
        IconsRole
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void load();

private:
    QVector<IconsModelData> m_data;
    QStringList previewIcons(const QString &name) const;
};
