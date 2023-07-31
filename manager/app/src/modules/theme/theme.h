#pragma once

#include <QObject>
#include <QString>
#include <QQmlEngine>

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include <MauiMan4/thememanager.h>

class Theme;
class IconsProxyModel;


struct KColorSchemeModelData {
    QString id; // e.g. BreezeDark
    QString name; // e.g. "Breeze Dark" or "Breeze-Dunkel"
    QString path;
    QStringList preview;
};

class ColorSchemesModelProxy : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged RESET resetFilter)

public:
    explicit ColorSchemesModelProxy(QObject *parent = nullptr);
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

class ColorSchemesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRole,
        PreviewRole,
        PathRole,
        FileRole
    };

    explicit ColorSchemesModel(QObject *parent=nullptr);

    int rowCount(const QModelIndex &parent) const override final;
    QVariant data(const QModelIndex &index, int role) const override final;
    QHash<int, QByteArray> roleNames() const override final;

private:
    mutable QList<KColorSchemeModelData> m_data;
    QStringList schemeColors(const QString &scheme) const;
};

class Decoration
{
public:
    QString name;
    QString source;
};

class WindowDecorationsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        Name,
        Source,
    };

    explicit WindowDecorationsModel(Theme *parent);

    int rowCount(const QModelIndex &parent) const override final;
    QVariant data(const QModelIndex &index, int role) const override final;
    QHash<int, QByteArray> roleNames() const override final;

private:
    QVector<Decoration> m_decos;

    void getDecorations();
};

class Theme : public MauiMan::ThemeManager
{
    Q_OBJECT
    Q_PROPERTY(WindowDecorationsModel* windowDecorationsModel READ windowDecorationsModel CONSTANT FINAL)
    Q_PROPERTY(IconsProxyModel* iconsModel READ iconsModel CONSTANT FINAL)
    Q_PROPERTY(ColorSchemesModelProxy* colorSchemeModel READ colorSchemeModel CONSTANT FINAL)
    Q_PROPERTY(QString wallpaper READ wallpaper NOTIFY wallpaperChanged FINAL)

public:
    explicit Theme(QObject * parent = nullptr);
    WindowDecorationsModel* windowDecorationsModel() ;
    IconsProxyModel * iconsModel();
    ColorSchemesModelProxy *colorSchemeModel();

    QString wallpaper() const;

Q_SIGNALS:
    void wallpaperChanged(QString wallpaper);

private:
    mutable WindowDecorationsModel *m_windowDecorationsModel;
    mutable IconsProxyModel *m_iconsModel;
    mutable ColorSchemesModelProxy *m_colorSchemeModel;

    QString m_wallpaper;
};

