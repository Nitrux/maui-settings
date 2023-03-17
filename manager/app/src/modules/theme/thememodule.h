#pragma once

#include "code/abstractmodule.h"
#include <QFont>

#include <QAbstractListModel>


struct KColorSchemeModelData {
    QString id; // e.g. BreezeDark
    QString name; // e.g. "Breeze Dark" or "Breeze-Dunkel"
    QString path;
    QStringList preview;
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


class Theme;
class ThemeModule : public AbstractModule
{
    Q_OBJECT
    Q_PROPERTY(Theme* manager READ manager CONSTANT FINAL)
    Q_PROPERTY(ColorSchemesModel* colorSchemeModel READ colorSchemeModel CONSTANT FINAL)

public:
    ThemeModule(QObject * parent = nullptr);
    Theme* manager();

    ColorSchemesModel* colorSchemeModel();

public slots:
    QFont getFont(const QString &desc);
    QString fontToString(const QFont &font);
    QStringList fontStyles(const QFont &font);
    QStringList fontPointSizes(const QFont &font);

private:
    Theme *m_manager;
    void updateGtk3Config();
    ColorSchemesModel* m_colorSchemeModel;
};

