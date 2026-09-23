#include "wallpapercontroller.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>
#include <utility>

namespace
{
const QSet<QString> imageSuffixes = {
    QStringLiteral("avif"),
    QStringLiteral("bmp"),
    QStringLiteral("gif"),
    QStringLiteral("heic"),
    QStringLiteral("heif"),
    QStringLiteral("jpeg"),
    QStringLiteral("jpg"),
    QStringLiteral("jxl"),
    QStringLiteral("png"),
    QStringLiteral("svg"),
    QStringLiteral("svgz"),
    QStringLiteral("tif"),
    QStringLiteral("tiff"),
    QStringLiteral("webp")
};
}

WallpaperController::WallpaperController(QObject *parent)
    : QAbstractListModel(parent)
    , m_systemWallpaperPath(QStringLiteral("/usr/share/wallpapers"))
    , m_picturesPath(defaultPicturesPath())
{
    refresh();
}

QString WallpaperController::systemWallpaperPath() const
{
    return m_systemWallpaperPath;
}

QString WallpaperController::picturesPath() const
{
    return m_picturesPath;
}

int WallpaperController::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant WallpaperController::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};

    const Entry &entry = m_entries.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
    case NameRole:
        return entry.name;
    case PathRole:
        return entry.path;
    case SourceRole:
        return entry.source;
    default:
        return {};
    }
}

QHash<int, QByteArray> WallpaperController::roleNames() const
{
    return {
        {PathRole, "path"},
        {NameRole, "name"},
        {SourceRole, "source"}
    };
}

void WallpaperController::refresh()
{
    const QString picturesPath = defaultPicturesPath();
    const bool picturesLocationChanged = m_picturesPath != picturesPath;
    m_picturesPath = picturesPath;

    QHash<QString, QString> systemEntries;
    QHash<QString, qint64> systemScores;
    QDir systemDirectory(m_systemWallpaperPath);
    QDirIterator systemIterator(m_systemWallpaperPath,
                                QDir::Files | QDir::Readable,
                                QDirIterator::Subdirectories);
    while (systemIterator.hasNext())
    {
        const QString path = systemIterator.next();
        if (!isImageFile(path))
            continue;

        const QString relativePath = systemDirectory.relativeFilePath(path);
        const QStringList components = relativePath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        const QString packageName = components.isEmpty()
            ? QFileInfo(path).completeBaseName()
            : components.constFirst();
        const qint64 score = imageScore(path);

        if (!systemEntries.contains(packageName) || score > systemScores.value(packageName))
        {
            systemEntries.insert(packageName, path);
            systemScores.insert(packageName, score);
        }
    }

    QStringList systemNames = systemEntries.keys();
    std::sort(systemNames.begin(), systemNames.end(), [](const QString &left, const QString &right) {
        return QString::localeAwareCompare(left, right) < 0;
    });

    QList<Entry> entries;
    QSet<QString> paths;
    for (const QString &name : std::as_const(systemNames))
    {
        const QString path = QDir::cleanPath(systemEntries.value(name));
        entries.append({path, name, QStringLiteral("system")});
        paths.insert(path);
    }

    QStringList picturePaths;
    QDirIterator picturesIterator(m_picturesPath,
                                  QDir::Files | QDir::Readable,
                                  QDirIterator::Subdirectories);
    while (picturesIterator.hasNext())
    {
        const QString path = picturesIterator.next();
        if (isImageFile(path))
            picturePaths.append(QDir::cleanPath(path));
    }

    std::sort(picturePaths.begin(), picturePaths.end(), [](const QString &left, const QString &right) {
        return QString::localeAwareCompare(left, right) < 0;
    });

    for (const QString &path : std::as_const(picturePaths))
    {
        if (paths.contains(path))
            continue;

        entries.append({path, QFileInfo(path).completeBaseName(), QStringLiteral("pictures")});
        paths.insert(path);
    }

    beginResetModel();
    m_entries = std::move(entries);
    endResetModel();

    if (picturesLocationChanged)
        Q_EMIT picturesPathChanged();
}

QString WallpaperController::defaultPicturesPath()
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    return path.isEmpty() ? QDir::homePath() + QStringLiteral("/Pictures") : QDir::cleanPath(path);
}

bool WallpaperController::isImageFile(const QString &path)
{
    return imageSuffixes.contains(QFileInfo(path).suffix().toLower());
}

qint64 WallpaperController::imageScore(const QString &path)
{
    // Avoid probing every catalog image with Qt plugins; malformed ICC profiles can make libpng warn.
    return QFileInfo(path).size();
}
