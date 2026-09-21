#include "flatpakpermissionscontroller.h"

#include <KConfig>
#include <KConfigGroup>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>

namespace
{
const QString contextGroupName = QStringLiteral("Context");
const QString environmentGroupName = QStringLiteral("Environment");
const QString sessionBusGroupName = QStringLiteral("Session Bus Policy");
const QString systemBusGroupName = QStringLiteral("System Bus Policy");

QMap<QString, QString> installedFlatpakNames()
{
    QMap<QString, QString> names;
    const QString executable = QStandardPaths::findExecutable(QStringLiteral("flatpak"));
    if (executable.isEmpty())
        return names;

    for (const QString &scope : {QStringLiteral("--user"), QStringLiteral("--system")})
    {
        QProcess process;
        QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
        environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
        environment.insert(QStringLiteral("LANG"), QStringLiteral("C"));
        process.setProcessEnvironment(environment);
        process.start(executable, {QStringLiteral("list"), scope, QStringLiteral("--app"), QStringLiteral("--columns=application,name")});
        if (!process.waitForStarted(1000) || !process.waitForFinished(5000))
        {
            process.kill();
            process.waitForFinished();
            continue;
        }

        for (const QByteArray &line : process.readAllStandardOutput().split(10))
        {
            const QStringList fields = QString::fromLocal8Bit(line).split(QLatin1Char('\t'));
            const QString appId = fields.value(0).trimmed();
            const QString name = fields.value(1).trimmed();
            if (!appId.isEmpty() && !name.isEmpty())
                names.insert(appId, name);
        }
    }

    return names;
}

struct SimplePermission
{
    const char *section;
    const char *category;
    const char *name;
    const char *label;
    const char *explanation = nullptr;
};

const SimplePermission simplePermissions[] = {
    {"basic", "shared", "network", "Internet connection", "Allow the app to access the network."},
    {"shared", "shared", "ipc", "Inter-process communication"},
    {"sockets", "sockets", "x11", "X11 windowing system", "Allow the app to create windows using X11."},
    {"sockets", "sockets", "wayland", "Wayland windowing system", "Allow the app to create windows using Wayland."},
    {"sockets", "sockets", "fallback-x11", "Fallback to X11 windowing system"},
    {"basic", "sockets", "pulseaudio", "PulseAudio sound server", "Allow the app to use the system audio server."},
    {"sockets", "sockets", "session-bus", "Session bus access"},
    {"sockets", "sockets", "system-bus", "System bus access"},
    {"basic", "sockets", "ssh-auth", "Remote login access"},
    {"basic", "sockets", "pcsc", "Smart card access"},
    {"basic", "sockets", "cups", "Print system access", "Allow the app to access system printers."},
    {"devices", "devices", "kvm", "Kernel-based Virtual Machine access"},
    {"devices", "devices", "dri", "Direct graphic rendering", "Allow the app to access the graphics hardware directly."},
    {"devices", "devices", "shm", "Host dev/shm"},
    {"basic", "devices", "all", "Device access"},
    {"features", "features", "devel", "System calls by development tools"},
    {"features", "features", "multiarch", "Run multiarch/multilib binaries"},
    {"basic", "features", "bluetooth", "Bluetooth", "Allow the app to communicate with Bluetooth devices."},
    {"features", "features", "canbus", "Canbus socket access"},
    {"features", "features", "per-app-dev-shm", "Share dev/shm across app instances"},
};

const struct
{
    const char *name;
    const char *label;
    const char *explanation = nullptr;
} standardFilesystems[] = {
    {"home", "All user files", "Allow the app to access files in your home directory."},
    {"host", "All system files", "Allow the app to access files throughout the system."},
    {"host-os", "System libraries, executables and binaries", "Allow the app to access system libraries and executables."},
    {"host-etc", "System configuration files", "Allow the app to access system-wide configuration files."},
};

QStringList uniqueSorted(const QSet<QString> &values)
{
    QStringList result = values.values();
    std::sort(result.begin(), result.end(), [](const QString &left, const QString &right) {
        return left.localeAwareCompare(right) < 0;
    });
    return result;
}

bool isListContextCategory(const QString &category)
{
    return category == QStringLiteral("shared")
        || category == QStringLiteral("sockets")
        || category == QStringLiteral("devices")
        || category == QStringLiteral("features")
        || category == QStringLiteral("filesystems")
        || category == QStringLiteral("persistent")
        || category == QStringLiteral("unset-environment");
}

void mergeConfig(KConfig &target, const QString &path)
{
    if (!QFileInfo::exists(path))
        return;

    const KConfig source(path, KConfig::SimpleConfig);
    for (const QString &groupName : source.groupList())
    {
        const KConfigGroup sourceGroup = source.group(groupName);
        KConfigGroup targetGroup = target.group(groupName);

        for (const QString &key : sourceGroup.keyList())
        {
            if (groupName == contextGroupName && isListContextCategory(key))
            {
                QStringList entries = targetGroup.readXdgListEntry(key);
                const QStringList sourceEntries = sourceGroup.readXdgListEntry(key);

                for (const QString &sourceEntry : sourceEntries)
                {
                    const QString sourceName = FlatpakPermissionsController::normalizeContextEntry(sourceEntry, key);
                    entries.erase(std::remove_if(entries.begin(), entries.end(), [&](const QString &entry) {
                        return FlatpakPermissionsController::normalizeContextEntry(entry, key) == sourceName;
                    }), entries.end());
                    entries.append(sourceEntry);
                }

                targetGroup.writeXdgListEntry(key, entries, KConfig::WriteConfigFlags());
            }
            else
            {
                targetGroup.writeEntry(key, sourceGroup.readEntry(key));
            }
        }
    }
}

void mergeConfigFiles(KConfig &config, const QStringList &paths)
{
    for (const QString &path : paths)
        mergeConfig(config, path);
}

QMap<QString, bool> simpleValues(const KConfigGroup &group, const QString &category)
{
    QMap<QString, bool> values;
    for (const QString &entry : group.readXdgListEntry(category))
    {
        QString name = entry;
        bool enabled = true;
        if (name.startsWith(QLatin1Char('!')))
        {
            name.remove(0, 1);
            enabled = false;
        }
        if (!name.isEmpty())
            values.insert(name, enabled);
    }
    return values;
}

QMap<QString, QString> filesystemValues(const KConfigGroup &group)
{
    QMap<QString, QString> values;
    for (const QString &entry : group.readXdgListEntry(QStringLiteral("filesystems")))
    {
        QString name;
        const QString mode = FlatpakPermissionsController::filesystemMode(entry, &name);
        if (!name.isEmpty())
            values.insert(name, mode);
    }
    return values;
}

QMap<QString, QString> groupValues(const KConfig &config, const QString &groupName)
{
    QMap<QString, QString> values;
    const KConfigGroup group = config.group(groupName);
    for (const QString &key : group.keyList())
        values.insert(key, group.readEntry(key));
    return values;
}

QVariantMap permissionEntry(const QString &section,
                            const QString &type,
                            const QString &name,
                            const QString &label,
                            const QString &explanation,
                            bool enabled,
                            bool defaultEnabled,
                            const QString &value,
                            const QString &defaultValue,
                            bool custom)
{
    return {
        {QStringLiteral("section"), section},
        {QStringLiteral("type"), type},
        {QStringLiteral("name"), name},
        {QStringLiteral("label"), label},
        {QStringLiteral("explanation"), explanation},
        {QStringLiteral("enabled"), enabled},
        {QStringLiteral("defaultEnabled"), defaultEnabled},
        {QStringLiteral("canBeDisabled"), true},
        {QStringLiteral("value"), value},
        {QStringLiteral("defaultValue"), defaultValue},
        {QStringLiteral("custom"), custom},
    };
}

bool samePermission(const QVariantMap &left, const QVariantMap &right)
{
    return left.value(QStringLiteral("section")) == right.value(QStringLiteral("section"))
        && left.value(QStringLiteral("name")) == right.value(QStringLiteral("name"))
        && left.value(QStringLiteral("enabled")) == right.value(QStringLiteral("enabled"))
        && left.value(QStringLiteral("value")) == right.value(QStringLiteral("value"));
}
}

FlatpakPermissionsController::FlatpakPermissionsController(QObject *parent)
    : QObject(parent)
{
    reload();
}

bool FlatpakPermissionsController::available() const
{
    return m_available;
}

QVariantList FlatpakPermissionsController::applications() const
{
    return m_applications;
}

QString FlatpakPermissionsController::selectedAppId() const
{
    return m_selectedAppId;
}

QString FlatpakPermissionsController::selectedAppName() const
{
    return m_selectedAppName;
}

QVariantList FlatpakPermissionsController::permissions() const
{
    return m_permissions;
}

bool FlatpakPermissionsController::isSaveNeeded() const
{
    if (m_permissions.size() != m_savedPermissions.size())
        return true;

    for (const QVariant &permissionValue : m_permissions)
    {
        const QVariantMap permission = permissionValue.toMap();
        bool found = false;
        for (const QVariant &savedValue : m_savedPermissions)
        {
            if (samePermission(permission, savedValue.toMap()))
            {
                found = true;
                break;
            }
        }
        if (!found)
            return true;
    }
    return false;
}

bool FlatpakPermissionsController::isDefaults() const
{
    for (const QVariant &permissionValue : m_permissions)
    {
        const QVariantMap permission = permissionValue.toMap();
        bool found = false;
        for (const QVariant &defaultValue : m_defaultPermissions)
        {
            const QVariantMap defaultPermissionValue = defaultValue.toMap();
            if (permissionKey(permission) == permissionKey(defaultPermissionValue))
            {
                found = true;
                if (!samePermission(permission, defaultPermissionValue))
                    return false;
                break;
            }
        }
        if (!found)
            return false;
    }
    return true;
}

QString FlatpakPermissionsController::errorMessage() const
{
    return m_errorMessage;
}

QStringList FlatpakPermissionsController::flatpakBaseDirectories()
{
    QStringList directories;

    const QString userDirectory = qEnvironmentVariable("FLATPAK_USER_DIR");
    directories.append(userDirectory.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/flatpak")
        : userDirectory);

    const QString systemDirectory = qEnvironmentVariable("FLATPAK_SYSTEM_DIR");
    if (!systemDirectory.isEmpty())
        directories.append(systemDirectory);
    else
    {
        directories.append(QStringLiteral("/var/lib/flatpak"));
        for (const QString &dataDirectory : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation))
            directories.append(dataDirectory + QStringLiteral("/flatpak"));
    }

    directories.removeDuplicates();
    return directories;
}

QString FlatpakPermissionsController::permissionKey(const QVariantMap &permission)
{
    return permission.value(QStringLiteral("section")).toString()
        + QLatin1Char('\x1f')
        + permission.value(QStringLiteral("name")).toString();
}

QString FlatpakPermissionsController::normalizeContextEntry(const QString &entry, const QString &category)
{
    QString name = entry;
    if (name.startsWith(QLatin1Char('!')))
        name.remove(0, 1);

    if (category == QStringLiteral("filesystems"))
    {
        if (name.endsWith(QStringLiteral(":ro")))
            name.chop(3);
        else if (name.endsWith(QStringLiteral(":rw")))
            name.chop(3);
        else if (name.endsWith(QStringLiteral(":create")))
            name.chop(7);
    }
    return name;
}

QString FlatpakPermissionsController::filesystemMode(const QString &entry, QString *name)
{
    QString value = entry;
    if (value.startsWith(QLatin1Char('!')))
    {
        value.remove(0, 1);
        if (name)
            *name = value;
        return QStringLiteral("off");
    }

    QString mode = QStringLiteral("rw");
    if (value.endsWith(QStringLiteral(":ro")))
    {
        value.chop(3);
        mode = QStringLiteral("ro");
    }
    else if (value.endsWith(QStringLiteral(":create")))
    {
        value.chop(7);
        mode = QStringLiteral("create");
    }
    else if (value.endsWith(QStringLiteral(":rw")))
        value.chop(3);

    if (name)
        *name = value;
    return mode;
}

QString FlatpakPermissionsController::formatFilesystemEntry(const QString &name, const QString &mode)
{
    if (mode == QStringLiteral("off"))
        return QLatin1Char('!') + name;
    if (mode == QStringLiteral("ro"))
        return name + QStringLiteral(":ro");
    if (mode == QStringLiteral("create"))
        return name + QStringLiteral(":create");
    return name;
}

QVariantList FlatpakPermissionsController::buildPermissions(const KConfig &defaults, const KConfig &effective)
{
    const KConfigGroup defaultContext = defaults.group(contextGroupName);
    const KConfigGroup effectiveContext = effective.group(contextGroupName);
    QVariantList permissions;

    const auto appendSimplePermission = [&](const SimplePermission &spec) {
        const QMap<QString, bool> defaultValues = simpleValues(defaultContext, QString::fromLatin1(spec.category));
        const QMap<QString, bool> effectiveValues = simpleValues(effectiveContext, QString::fromLatin1(spec.category));
        const bool defaultEnabled = defaultValues.value(QString::fromLatin1(spec.name), false);
        const bool enabled = effectiveValues.value(QString::fromLatin1(spec.name), defaultEnabled);
        permissions.append(permissionEntry(QString::fromLatin1(spec.section),
                                            QStringLiteral("simple"),
                                            QString::fromLatin1(spec.name),
                                            QString::fromLatin1(spec.label),
                                            spec.explanation
                                                ? QString::fromLatin1(spec.explanation)
                                                : QStringLiteral("Allow this Flatpak capability."),
                                            enabled,
                                            defaultEnabled,
                                            {},
                                            {},
                                            false));
    };

    for (const SimplePermission &spec : simplePermissions)
        appendSimplePermission(spec);

    const QMap<QString, QString> defaultFilesystems = filesystemValues(defaultContext);
    const QMap<QString, QString> effectiveFilesystems = filesystemValues(effectiveContext);
    QSet<QString> filesystemNames;
    for (auto it = defaultFilesystems.cbegin(); it != defaultFilesystems.cend(); ++it)
        filesystemNames.insert(it.key());
    for (auto it = effectiveFilesystems.cbegin(); it != effectiveFilesystems.cend(); ++it)
        filesystemNames.insert(it.key());
    for (const auto &filesystem : standardFilesystems)
        filesystemNames.insert(QString::fromLatin1(filesystem.name));

    for (const QString &name : uniqueSorted(filesystemNames))
    {
        if (name == QStringLiteral("xdg-config/kdeglobals"))
            continue;

        QString label = name;
        QString explanation = QStringLiteral("Choose how this path is exposed to the application.");
        for (const auto &filesystem : standardFilesystems)
        {
            if (name == QString::fromLatin1(filesystem.name))
            {
                label = QString::fromLatin1(filesystem.label);
                if (filesystem.explanation)
                    explanation = QString::fromLatin1(filesystem.explanation);
                break;
            }
        }

        const QString defaultMode = defaultFilesystems.value(name, QStringLiteral("off"));
        const QString mode = effectiveFilesystems.value(name, defaultMode);
        permissions.append(permissionEntry(QStringLiteral("filesystem"),
                                            QStringLiteral("filesystem"),
                                            name,
                                            label,
                                            explanation,
                                            mode != QStringLiteral("off"),
                                            defaultMode != QStringLiteral("off"),
                                            mode,
                                            defaultMode,
                                            !defaultFilesystems.contains(name)));
    }

    const auto appendPolicyPermissions = [&](const QString &section, const QString &groupName) {
        const QMap<QString, QString> defaultValues = groupValues(defaults, groupName);
        const QMap<QString, QString> effectiveValues = groupValues(effective, groupName);
        QSet<QString> names;
        for (auto it = defaultValues.cbegin(); it != defaultValues.cend(); ++it)
            names.insert(it.key());
        for (auto it = effectiveValues.cbegin(); it != effectiveValues.cend(); ++it)
            names.insert(it.key());

        for (const QString &name : uniqueSorted(names))
        {
            const QString defaultValue = defaultValues.value(name, QStringLiteral("none"));
            const QString value = effectiveValues.value(name, defaultValue);
            permissions.append(permissionEntry(section,
                                                QStringLiteral("bus"),
                                                name,
                                                name,
                                                QStringLiteral("Choose the D-Bus policy for this service."),
                                                true,
                                                true,
                                                value,
                                                defaultValue,
                                                !defaultValues.contains(name)));
        }
    };

    appendPolicyPermissions(QStringLiteral("session-bus"), sessionBusGroupName);
    appendPolicyPermissions(QStringLiteral("system-bus"), systemBusGroupName);

    const QMap<QString, QString> defaultEnvironment = groupValues(defaults, environmentGroupName);
    const QMap<QString, QString> effectiveEnvironment = groupValues(effective, environmentGroupName);
    QSet<QString> environmentNames;
    for (auto it = defaultEnvironment.cbegin(); it != defaultEnvironment.cend(); ++it)
        environmentNames.insert(it.key());
    for (auto it = effectiveEnvironment.cbegin(); it != effectiveEnvironment.cend(); ++it)
        environmentNames.insert(it.key());

    for (const QString &name : uniqueSorted(environmentNames))
    {
        const QString defaultValue = defaultEnvironment.value(name);
        const QString value = effectiveEnvironment.value(name, defaultValue);
        permissions.append(permissionEntry(QStringLiteral("environment"),
                                            QStringLiteral("environment"),
                                            name,
                                            name,
                                            QStringLiteral("Environment value passed to the application."),
                                            true,
                                            true,
                                            value,
                                            defaultValue,
                                            !defaultEnvironment.contains(name)));
    }

    return permissions;
}

void FlatpakPermissionsController::loadApplications()
{
    m_applicationData.clear();
    const QMap<QString, QString> installedNames = installedFlatpakNames();
    QMap<QString, Application> found;

    for (const QString &baseDirectory : flatpakBaseDirectories())
    {
        const QDir appDirectory(baseDirectory + QStringLiteral("/app"));
        if (!appDirectory.exists())
            continue;

        const QFileInfoList appEntries = appDirectory.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &appEntry : appEntries)
        {
            const QString metadataPath = appEntry.absoluteFilePath() + QStringLiteral("/current/active/metadata");
            if (!QFileInfo::exists(metadataPath))
                continue;

            const KConfig metadata(metadataPath, KConfig::SimpleConfig);
            const QString appId = appEntry.fileName();
            const QString metadataName = metadata.group(QStringLiteral("Application"))
                                             .readEntry(QStringLiteral("name"), appId);
            const QString displayName = installedNames.value(appId, metadataName);
            found.insert(appId, {appId, displayName, baseDirectory, metadataPath});
        }
    }

    for (auto it = found.cbegin(); it != found.cend(); ++it)
    {
        m_applicationData.append(it.value());
        m_applications.append(QVariantMap{
            {QStringLiteral("id"), it.value().appId},
            {QStringLiteral("name"), it.value().displayName},
            {QStringLiteral("icon"), it.value().appId},
        });
    }
}

void FlatpakPermissionsController::reload()
{
    const bool previousAvailable = m_available;
    const QString previousAppId = m_selectedAppId;
    const QString previousAppName = m_selectedAppName;
    const QVariantList previousApplications = m_applications;

    m_applications.clear();
    m_available = !QStandardPaths::findExecutable(QStringLiteral("flatpak")).isEmpty();
    loadApplications();
    for (const QString &baseDirectory : flatpakBaseDirectories())
        m_available = m_available || QFileInfo::exists(baseDirectory + QStringLiteral("/app"));

    if (m_available != previousAvailable)
        Q_EMIT availableChanged();
    if (m_applications != previousApplications)
        Q_EMIT applicationsChanged();

    QString nextAppId = previousAppId;
    bool stillInstalled = false;
    for (const QVariant &application : m_applications)
    {
        if (application.toMap().value(QStringLiteral("id")).toString() == nextAppId)
        {
            stillInstalled = true;
            break;
        }
    }
    if (!stillInstalled)
        nextAppId = m_applications.isEmpty() ? QString() : m_applications.first().toMap().value(QStringLiteral("id")).toString();

    if (nextAppId.isEmpty())
    {
        m_selectedAppId.clear();
        m_selectedAppName.clear();
        m_permissions.clear();
        m_defaultPermissions.clear();
        m_savedPermissions.clear();
        m_userOverridePath.clear();
        if (previousAppId != m_selectedAppId || previousAppName != m_selectedAppName)
            Q_EMIT selectedAppChanged();
        Q_EMIT permissionsChanged();
        setErrorMessage({});
        return;
    }

    if (nextAppId != m_selectedAppId)
        selectApplication(nextAppId);
    else
        loadSelectedApplication();
}

void FlatpakPermissionsController::selectApplication(const QString &appId)
{
    if (appId.isEmpty() || appId == m_selectedAppId)
    {
        if (!appId.isEmpty())
            loadSelectedApplication();
        return;
    }

    const auto it = std::find_if(m_applicationData.cbegin(), m_applicationData.cend(), [&](const Application &application) {
        return application.appId == appId;
    });
    if (it == m_applicationData.cend())
        return;

    m_selectedAppId = appId;
    m_selectedAppName = it->displayName;
    Q_EMIT selectedAppChanged();
    loadSelectedApplication();
}

void FlatpakPermissionsController::loadSelectedApplication()
{
    const auto it = std::find_if(m_applicationData.cbegin(), m_applicationData.cend(), [&](const Application &application) {
        return application.appId == m_selectedAppId;
    });
    if (it == m_applicationData.cend())
        return;

    const QString userBase = flatpakBaseDirectories().value(0);
    const QString systemOverrides = it->baseDirectory + QStringLiteral("/overrides");
    const QString userOverrides = userBase + QStringLiteral("/overrides");
    const QStringList defaultsFiles = {
        it->metadataPath,
        systemOverrides + QStringLiteral("/global"),
        systemOverrides + QLatin1Char('/') + it->appId,
        userOverrides + QStringLiteral("/global"),
    };
    m_userOverridePath = userOverrides + QLatin1Char('/') + it->appId;

    KConfig defaultsConfig(QString(), KConfig::SimpleConfig);
    KConfig effectiveConfig(QString(), KConfig::SimpleConfig);
    mergeConfigFiles(defaultsConfig, defaultsFiles);
    mergeConfigFiles(effectiveConfig, defaultsFiles + QStringList{m_userOverridePath});
    m_defaultPermissions = buildPermissions(defaultsConfig, defaultsConfig);
    m_permissions = buildPermissions(defaultsConfig, effectiveConfig);
    m_savedPermissions = m_permissions;
    Q_EMIT permissionsChanged();
    setErrorMessage({});
}

bool FlatpakPermissionsController::setPermission(int index, bool enabled)
{
    if (index < 0 || index >= m_permissions.size())
        return false;

    QVariantMap permission = m_permissions.at(index).toMap();
    if (!permission.value(QStringLiteral("canBeDisabled")).toBool() && !enabled)
        return false;

    permission.insert(QStringLiteral("enabled"), enabled);
    m_permissions[index] = permission;
    Q_EMIT permissionsChanged();
    return true;
}

bool FlatpakPermissionsController::setPermissionValue(int index, const QString &value)
{
    if (index < 0 || index >= m_permissions.size())
        return false;

    QVariantMap permission = m_permissions.at(index).toMap();
    const QString type = permission.value(QStringLiteral("type")).toString();
    if (type == QStringLiteral("filesystem")
        && !QStringList{QStringLiteral("ro"), QStringLiteral("rw"), QStringLiteral("create"), QStringLiteral("off")}.contains(value))
        return false;
    if (type == QStringLiteral("bus")
        && !QStringList{QStringLiteral("none"), QStringLiteral("see"), QStringLiteral("talk"), QStringLiteral("own")}.contains(value))
        return false;

    permission.insert(QStringLiteral("value"), value);
    if (type == QStringLiteral("filesystem"))
        permission.insert(QStringLiteral("enabled"), value != QStringLiteral("off"));
    m_permissions[index] = permission;
    Q_EMIT permissionsChanged();
    return true;
}

bool FlatpakPermissionsController::validPermissionName(const QString &section, const QString &name) const
{
    if (name.trimmed().isEmpty() || name.contains(QChar(10)) || name.contains(QChar(13)) || name.contains(QChar(';')))
        return false;

    if (section == QStringLiteral("filesystem"))
        return !name.startsWith(QLatin1Char('!')) && !name.contains(QStringLiteral(":ro"))
            && !name.contains(QStringLiteral(":rw")) && !name.contains(QStringLiteral(":create"));
    if (section == QStringLiteral("session-bus") || section == QStringLiteral("system-bus"))
        return QRegularExpression(QStringLiteral("^[A-Za-z_][A-Za-z0-9_-]*(\\.[A-Za-z_][A-Za-z0-9_-]*)*(\\.\\*)?$")).match(name).hasMatch();
    if (section == QStringLiteral("environment"))
        return QRegularExpression(QStringLiteral("^[A-Za-z_][A-Za-z0-9_]*$")).match(name).hasMatch();
    return false;
}

bool FlatpakPermissionsController::hasPermission(const QString &section, const QString &name) const
{
    for (const QVariant &permissionValue : m_permissions)
    {
        const QVariantMap permission = permissionValue.toMap();
        if (permission.value(QStringLiteral("section")).toString() == section
            && permission.value(QStringLiteral("name")).toString() == name)
            return true;
    }
    return false;
}

bool FlatpakPermissionsController::addPermission(const QString &section, const QString &name, const QString &value)
{
    const QString normalizedName = name.trimmed();
    if (!validPermissionName(section, normalizedName) || hasPermission(section, normalizedName))
    {
        setErrorMessage(QStringLiteral("Enter a valid, unique permission name."));
        return false;
    }

    QString type;
    QString normalizedValue = value;
    if (section == QStringLiteral("filesystem"))
    {
        type = QStringLiteral("filesystem");
        if (!QStringList{QStringLiteral("ro"), QStringLiteral("rw"), QStringLiteral("create"), QStringLiteral("off")}.contains(normalizedValue))
            normalizedValue = QStringLiteral("rw");
    }
    else if (section == QStringLiteral("session-bus") || section == QStringLiteral("system-bus"))
    {
        type = QStringLiteral("bus");
        if (!QStringList{QStringLiteral("none"), QStringLiteral("see"), QStringLiteral("talk"), QStringLiteral("own")}.contains(normalizedValue))
            normalizedValue = QStringLiteral("none");
    }
    else if (section == QStringLiteral("environment"))
    {
        type = QStringLiteral("environment");
        if (normalizedValue.contains(QChar(10)) || normalizedValue.contains(QChar(13)))
        {
            setErrorMessage(QStringLiteral("Enter a single-line environment value."));
            return false;
        }
    }
    else
        return false;

    const QString label = normalizedName;
    m_permissions.append(permissionEntry(section,
                                          type,
                                          normalizedName,
                                          label,
                                          type == QStringLiteral("environment")
                                              ? QStringLiteral("Environment value passed to the application.")
                                              : QStringLiteral("User-defined Flatpak permission."),
                                          normalizedValue != QStringLiteral("off"),
                                          false,
                                          normalizedValue,
                                          type == QStringLiteral("filesystem") ? QStringLiteral("off") : QString(),
                                          true));
    Q_EMIT permissionsChanged();
    setErrorMessage({});
    return true;
}

bool FlatpakPermissionsController::removePermission(int index)
{
    if (index < 0 || index >= m_permissions.size())
        return false;

    const QVariantMap permission = m_permissions.at(index).toMap();
    if (!permission.value(QStringLiteral("custom")).toBool())
        return false;

    m_permissions.removeAt(index);
    Q_EMIT permissionsChanged();
    return true;
}

void FlatpakPermissionsController::defaults()
{
    m_permissions = m_defaultPermissions;
    Q_EMIT permissionsChanged();
}

bool FlatpakPermissionsController::save()
{
    if (m_selectedAppId.isEmpty() || m_userOverridePath.isEmpty())
        return false;

    if (!isSaveNeeded())
        return true;

    if (isDefaults())
    {
        if (QFileInfo::exists(m_userOverridePath) && !QFile::remove(m_userOverridePath))
        {
            setErrorMessage(QStringLiteral("Could not remove the Flatpak permission overrides."));
            return false;
        }
        m_savedPermissions = m_permissions;
        Q_EMIT permissionsChanged();
        setErrorMessage({});
        return true;
    }

    const QFileInfo overrideInfo(m_userOverridePath);
    if (!QDir().mkpath(overrideInfo.absolutePath()))
    {
        setErrorMessage(QStringLiteral("Could not create the Flatpak overrides directory."));
        return false;
    }

    KConfig config(m_userOverridePath, KConfig::SimpleConfig);
    if (!config.isConfigWritable(true))
    {
        setErrorMessage(QStringLiteral("The Flatpak permission overrides are not writable."));
        return false;
    }
    for (const QString &group : config.groupList())
        config.deleteGroup(group);

    QMap<QString, QStringList> contextEntries;
    QMap<QString, QMap<QString, QString>> groupedEntries;
    for (const QVariant &permissionValue : m_permissions)
    {
        const QVariantMap permission = permissionValue.toMap();
        const QString type = permission.value(QStringLiteral("type")).toString();
        const QString section = permission.value(QStringLiteral("section")).toString();
        const QString name = permission.value(QStringLiteral("name")).toString();
        const QVariantMap defaultValue = defaultPermission(permission);

        if (type == QStringLiteral("simple"))
        {
            if (permission.value(QStringLiteral("enabled")).toBool()
                != defaultValue.value(QStringLiteral("enabled"), false).toBool())
                contextEntries[permission.value(QStringLiteral("section")).toString() == QStringLiteral("basic")
                                  ? (name == QStringLiteral("network") ? QStringLiteral("shared")
                                                                         : name == QStringLiteral("pulseaudio") || name == QStringLiteral("ssh-auth")
                                                                               || name == QStringLiteral("pcsc") || name == QStringLiteral("cups")
                                                                               ? QStringLiteral("sockets")
                                                                               : name == QStringLiteral("all") ? QStringLiteral("devices")
                                                                                                                : QStringLiteral("features"))
                                  : permission.value(QStringLiteral("section")).toString() == QStringLiteral("shared") ? QStringLiteral("shared")
                                  : permission.value(QStringLiteral("section")).toString()]
                    .append(permission.value(QStringLiteral("enabled")).toBool() ? name : QLatin1Char('!') + name);
        }
        else if (type == QStringLiteral("filesystem"))
        {
            const QString value = permission.value(QStringLiteral("value")).toString();
            const QString defaultMode = defaultValue.value(QStringLiteral("value"), QStringLiteral("off")).toString();
            if (value != defaultMode || permission.value(QStringLiteral("custom")).toBool())
            {
                if (value != QStringLiteral("off"))
                    contextEntries[QStringLiteral("filesystems")].append(formatFilesystemEntry(name, value));
                else if (defaultMode != QStringLiteral("off"))
                    contextEntries[QStringLiteral("filesystems")].append(formatFilesystemEntry(name, value));
            }
        }
        else if (type == QStringLiteral("bus"))
        {
            const QString value = permission.value(QStringLiteral("value")).toString();
            if (permission.value(QStringLiteral("custom")).toBool()
                || value != defaultValue.value(QStringLiteral("value"), QStringLiteral("none")).toString())
                groupedEntries[section == QStringLiteral("session-bus") ? sessionBusGroupName : systemBusGroupName].insert(name, value);
        }
        else if (type == QStringLiteral("environment"))
        {
            const QString value = permission.value(QStringLiteral("value")).toString();
            if (permission.value(QStringLiteral("custom")).toBool()
                || value != defaultValue.value(QStringLiteral("value")).toString())
                groupedEntries[environmentGroupName].insert(name, value);
        }
    }

    KConfigGroup context = config.group(contextGroupName);
    for (auto it = contextEntries.cbegin(); it != contextEntries.cend(); ++it)
    {
        if (!it.value().isEmpty())
            context.writeXdgListEntry(it.key(), it.value(), KConfig::WriteConfigFlags());
    }
    for (auto groupIt = groupedEntries.cbegin(); groupIt != groupedEntries.cend(); ++groupIt)
    {
        KConfigGroup group = config.group(groupIt.key());
        for (auto entryIt = groupIt.value().cbegin(); entryIt != groupIt.value().cend(); ++entryIt)
            group.writeEntry(entryIt.key(), entryIt.value());
    }
    config.sync();

    m_savedPermissions = m_permissions;
    Q_EMIT permissionsChanged();
    setErrorMessage({});
    return true;
}

void FlatpakPermissionsController::clearError()
{
    setErrorMessage({});
}

QVariantMap FlatpakPermissionsController::defaultPermission(const QVariantMap &permission) const
{
    const QString key = permissionKey(permission);
    for (const QVariant &defaultValue : m_defaultPermissions)
    {
        if (permissionKey(defaultValue.toMap()) == key)
            return defaultValue.toMap();
    }
    return {};
}

void FlatpakPermissionsController::setErrorMessage(const QString &message)
{
    if (m_errorMessage == message)
        return;

    m_errorMessage = message;
    Q_EMIT errorMessageChanged();
}
