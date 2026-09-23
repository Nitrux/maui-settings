#pragma once

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QVariant>

class WallpaperController final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString systemWallpaperPath READ systemWallpaperPath CONSTANT)
    Q_PROPERTY(QString picturesPath READ picturesPath NOTIFY picturesPathChanged)

public:
    enum Role
    {
        PathRole = Qt::UserRole + 1,
        NameRole,
        SourceRole
    };
    Q_ENUM(Role)

    explicit WallpaperController(QObject *parent = nullptr);

    QString systemWallpaperPath() const;
    QString picturesPath() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();

Q_SIGNALS:
    void picturesPathChanged();

private:
    struct Entry
    {
        QString path;
        QString name;
        QString source;
    };

    static QString defaultPicturesPath();
    static bool isImageFile(const QString &path);
    static qint64 imageScore(const QString &path);

    QString m_systemWallpaperPath;
    QString m_picturesPath;
    QList<Entry> m_entries;
};
