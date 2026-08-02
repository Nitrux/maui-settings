#include "mimecontroller.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>

namespace
{
constexpr auto defaultApplicationsGroup = "[Default Applications]";

struct MimeAppsDocument
{
    QStringList lines;
    QString newline = QStringLiteral("\n");
};

MimeAppsDocument readDocument(const QByteArray &data)
{
    MimeAppsDocument document;
    document.newline = data.contains("\r\n") ? QStringLiteral("\r\n") : QStringLiteral("\n");
    document.lines = QString::fromUtf8(data).split(QLatin1Char('\n'), Qt::KeepEmptyParts);

    if (document.newline == QStringLiteral("\r\n"))
    {
        for (QString &line : document.lines)
        {
            if (line.endsWith(QLatin1Char('\r')))
                line.chop(1);
        }
    }

    return document;
}

QByteArray serializeDocument(const MimeAppsDocument &document)
{
    return document.lines.join(document.newline).toUtf8();
}

bool isGroupHeader(const QString &line)
{
    const QString trimmed = line.trimmed();
    return trimmed.startsWith(QLatin1Char('[')) && trimmed.endsWith(QLatin1Char(']'));
}

int defaultGroupStart(const QStringList &lines)
{
    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines.at(i).trimmed() == QLatin1String(defaultApplicationsGroup))
            return i;
    }

    return -1;
}

int groupEnd(const QStringList &lines, int groupStart)
{
    for (int i = groupStart + 1; i < lines.size(); ++i)
    {
        if (isGroupHeader(lines.at(i)))
            return i;
    }

    return lines.size();
}

bool parseAssociation(const QString &line, QString *mimeType, QString *application)
{
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#')) || trimmed.startsWith(QLatin1Char(';')))
        return false;

    const qsizetype separator = line.indexOf(QLatin1Char('='));
    if (separator <= 0)
        return false;

    *mimeType = line.left(separator).trimmed();

    const QStringList applications = line.mid(separator + 1).trimmed().split(QLatin1Char(';'), Qt::SkipEmptyParts);
    QStringList displayApplications;
    displayApplications.reserve(applications.size());
    for (const QString &item : applications)
    {
        const QString value = item.trimmed();
        if (!value.isEmpty())
            displayApplications.append(value);
    }

    *application = displayApplications.join(QStringLiteral("; "));
    return !mimeType->isEmpty() && !application->isEmpty();
}

QString normalizedApplication(const QString &input)
{
    QString value = input.trimmed();
    if (value.startsWith(QStringLiteral("file:")))
        value = QUrl(value).toLocalFile();

    const QStringList values = value.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    QStringList normalized;
    normalized.reserve(values.size());
    for (const QString &item : values)
    {
        const QString trimmed = item.trimmed();
        if (!trimmed.isEmpty())
            normalized.append(trimmed);
    }

    return normalized.join(QLatin1Char(';')) + (normalized.isEmpty() ? QString() : QStringLiteral(";"));
}

bool isValidMimeType(const QString &mimeType)
{
    static const QRegularExpression expression(
        QStringLiteral(R"(^[A-Za-z0-9][A-Za-z0-9!#$&^_.+\-]*/[A-Za-z0-9][A-Za-z0-9!#$&^_.+\-]*$)"));
    return expression.match(mimeType).hasMatch();
}
} // namespace

MimeController::MimeController(QObject *parent)
    : QObject(parent)
    , m_filePath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/mimeapps.list"))
{
    reload();
}

QVariantList MimeController::associations() const
{
    return m_associations;
}

QString MimeController::filePath() const
{
    return m_filePath;
}

QString MimeController::errorMessage() const
{
    return m_errorMessage;
}

void MimeController::reload()
{
    QVariantList associations;
    QFile file(m_filePath);

    if (file.exists() && !file.open(QIODevice::ReadOnly))
    {
        setErrorMessage(tr("Could not open %1: %2").arg(m_filePath, file.errorString()));
        return;
    }

    if (file.isOpen())
    {
        const MimeAppsDocument document = readDocument(file.readAll());
        const int start = defaultGroupStart(document.lines);
        if (start >= 0)
        {
            const int end = groupEnd(document.lines, start);
            for (int i = start + 1; i < end; ++i)
            {
                QString mimeType;
                QString application;
                if (!parseAssociation(document.lines.at(i), &mimeType, &application))
                    continue;

                QVariantMap association;
                association.insert(QStringLiteral("mimeType"), mimeType);
                association.insert(QStringLiteral("application"), application);
                associations.append(association);
            }
        }
    }

    const bool changed = associations != m_associations;
    m_associations = associations;
    setErrorMessage({});

    if (changed)
        Q_EMIT associationsChanged();
}

bool MimeController::setAssociation(const QString &mimeType, const QString &application)
{
    const QString normalizedMimeType = mimeType.trimmed();
    const QString normalizedValue = normalizedApplication(application);

    if (!isValidMimeType(normalizedMimeType))
    {
        setErrorMessage(tr("Enter a valid MIME type, for example text/plain."));
        return false;
    }

    if (normalizedValue.isEmpty() || normalizedValue.contains(QLatin1Char('\n')) || normalizedValue.contains(QLatin1Char('\r')))
    {
        setErrorMessage(tr("Enter a desktop file ID or executable path."));
        return false;
    }

    return writeAssociation(normalizedMimeType, normalizedValue, false);
}

bool MimeController::removeAssociation(const QString &mimeType)
{
    const QString normalizedMimeType = mimeType.trimmed();
    if (normalizedMimeType.isEmpty())
    {
        setErrorMessage(tr("No MIME type was selected."));
        return false;
    }

    return writeAssociation(normalizedMimeType, {}, true);
}

void MimeController::clearError()
{
    setErrorMessage({});
}

bool MimeController::writeAssociation(const QString &mimeType, const QString &application, bool remove)
{
    QByteArray originalData;
    QFile source(m_filePath);
    QFileDevice::Permissions originalPermissions;

    if (source.exists())
    {
        originalPermissions = source.permissions();
        if (!source.open(QIODevice::ReadOnly))
        {
            setErrorMessage(tr("Could not open %1: %2").arg(m_filePath, source.errorString()));
            return false;
        }
        originalData = source.readAll();
    }

    MimeAppsDocument document = readDocument(originalData);
    if (originalData.isEmpty())
        document.lines.clear();

    int start = defaultGroupStart(document.lines);
    if (start < 0)
    {
        if (remove)
        {
            setErrorMessage({});
            return true;
        }

        if (!document.lines.isEmpty() && !document.lines.constLast().isEmpty())
            document.lines.append(QString());

        start = document.lines.size();
        document.lines.append(QLatin1String(defaultApplicationsGroup));
    }

    int end = groupEnd(document.lines, start);
    int firstMatch = -1;
    for (int i = start + 1; i < end; ++i)
    {
        const QString line = document.lines.at(i);
        const qsizetype separator = line.indexOf(QLatin1Char('='));
        if (separator > 0 && line.left(separator).trimmed() == mimeType)
        {
            if (firstMatch < 0)
                firstMatch = i;
            else
            {
                document.lines.removeAt(i);
                --i;
                --end;
            }
        }
    }

    if (remove)
    {
        if (firstMatch >= 0)
            document.lines.removeAt(firstMatch);
        else
        {
            setErrorMessage({});
            return true;
        }
    }
    else
    {
        const QString replacement = mimeType + QLatin1Char('=') + application;
        if (firstMatch >= 0)
            document.lines[firstMatch] = replacement;
        else
            document.lines.insert(end, replacement);
    }

    if (document.lines.isEmpty() || !document.lines.constLast().isEmpty())
        document.lines.append(QString());

    const QFileInfo fileInfo(m_filePath);
    QDir directory(fileInfo.absolutePath());
    if (!directory.exists() && !directory.mkpath(QStringLiteral(".")))
    {
        setErrorMessage(tr("Could not create %1.").arg(fileInfo.absolutePath()));
        return false;
    }

    QSaveFile destination(m_filePath);
    if (!destination.open(QIODevice::WriteOnly))
    {
        setErrorMessage(tr("Could not write %1: %2").arg(m_filePath, destination.errorString()));
        return false;
    }

    if (originalPermissions != QFileDevice::Permissions())
        destination.setPermissions(originalPermissions);

    const QByteArray output = serializeDocument(document);
    if (destination.write(output) != output.size())
    {
        setErrorMessage(tr("Could not write %1: %2").arg(m_filePath, destination.errorString()));
        destination.cancelWriting();
        return false;
    }

    if (!destination.commit())
    {
        setErrorMessage(tr("Could not replace %1: %2").arg(m_filePath, destination.errorString()));
        return false;
    }

    reload();
    return true;
}

void MimeController::setErrorMessage(const QString &message)
{
    if (m_errorMessage == message)
        return;

    m_errorMessage = message;
    Q_EMIT errorMessageChanged();
}
