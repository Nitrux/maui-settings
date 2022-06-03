#pragma once

#include <QObject>
#include <QString>
#include <QQmlEngine>
#include <QAbstractListModel>

#include <MauiMan/thememanager.h>

class Theme;

class Decoration
{
public:
    QString name;
    QString source;
    int radius;
};

class WindowDecorationsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles
    {
        Name,
        Source,
        Radius
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

public:
    explicit Theme(QObject * parent = nullptr);    
    WindowDecorationsModel* windowDecorationsModel() const;

private:
    WindowDecorationsModel* m_windowDecorationsModel;

};

