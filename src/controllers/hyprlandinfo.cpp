#include "hyprlandinfo.h"
#include "hyprlanddevices.h"
#include "hyprlandkeybinds.h"
#include "hyprlandrules.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QProcess>
#include <QtGlobal>

namespace
{
struct TableRange
{
    int start = -1;
    int end = -1;
};

QString hyprlandConfigPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + QStringLiteral("/hypr/hyprland.lua");
}

int braceDelta(const QString &line)
{
    bool quoted = false;
    bool escaped = false;
    int delta = 0;

    for (int i = 0; i < line.size(); ++i)
    {
        const QChar character = line.at(i);
        if (!quoted && character == QLatin1Char('-') && i + 1 < line.size()
            && line.at(i + 1) == QLatin1Char('-'))
            break;

        if (character == QLatin1Char('"') && !escaped)
            quoted = !quoted;
        if (!quoted)
        {
            if (character == QLatin1Char('{'))
                ++delta;
            else if (character == QLatin1Char('}'))
                --delta;
        }
        escaped = character == QLatin1Char('\\') && !escaped;
        if (character != QLatin1Char('\\'))
            escaped = false;
    }

    return delta;
}

TableRange findTable(const QStringList &lines, const QString &key, int firstLine = 0, int lastLine = -1)
{
    if (lastLine < 0 || lastLine >= lines.size())
        lastLine = lines.size() - 1;

    const QRegularExpression startExpression(
        QStringLiteral("^\\s*%1\\s*=\\s*\\{").arg(QRegularExpression::escape(key)));
    for (int start = qMax(0, firstLine); start <= lastLine; ++start)
    {
        if (!startExpression.match(lines.at(start)).hasMatch())
            continue;

        int depth = 0;
        for (int line = start; line <= lastLine; ++line)
        {
            depth += braceDelta(lines.at(line));
            if (depth == 0)
                return {start, line};
        }
    }

    return {};
}

QString valueInTable(const QStringList &lines, const TableRange &table, const QString &key)
{
    if (table.start < 0 || table.end < table.start)
        return {};

    const QRegularExpression expression(
        QStringLiteral("\\b%1\\s*=\\s*([^,}\\n]+)").arg(QRegularExpression::escape(key)));
    for (int line = table.start; line <= table.end; ++line)
    {
        const auto match = expression.match(lines.at(line));
        if (match.hasMatch())
            return match.captured(1).trimmed().remove(QLatin1Char('"'));
    }

    return {};
}

TableRange findGesture(const QStringList &lines, const QString &wantedDirection, const QString &wantedAction)
{
    const QString direction = wantedDirection.trimmed().toLower();
    const QString action = wantedAction.trimmed().toLower();
    const QRegularExpression startExpression(QStringLiteral("^\\s*hl\\.gesture\\s*\\(\\s*\\{"));
    for (int start = 0; start < lines.size(); ++start)
    {
        if (!startExpression.match(lines.at(start)).hasMatch())
            continue;

        int depth = 0;
        for (int end = start; end < lines.size(); ++end)
        {
            depth += braceDelta(lines.at(end));
            if (depth != 0)
                continue;

            const TableRange gesture{start, end};
            if (valueInTable(lines, gesture, QStringLiteral("direction")).trimmed().toLower() == direction
                && valueInTable(lines, gesture, QStringLiteral("action")).trimmed().toLower() == action)
                return gesture;
            break;
        }
    }

    return {};
}

TableRange findWorkspaceSwipeGesture(const QStringList &lines)
{
    return findGesture(lines, QStringLiteral("horizontal"), QStringLiteral("workspace"));
}

bool replaceValue(QStringList &lines, const TableRange &table, const QString &key, const QString &value)
{
    if (table.start < 0 || table.end < table.start)
        return false;

    const QRegularExpression expression(
        QStringLiteral("(\\b%1\\s*=\\s*)([^,}\\n]+)").arg(QRegularExpression::escape(key)));
    for (int line = table.start; line <= table.end; ++line)
    {
        auto match = expression.match(lines.at(line));
        if (!match.hasMatch())
            continue;

        lines[line].replace(match.capturedStart(2), match.capturedLength(2), value);
        return true;
    }

    return false;
}

QString localValue(const QStringList &lines, const QString &key)
{
    const QRegularExpression expression(
        QStringLiteral("^\\s*local\\s+%1\\s*=\\s*\"([^\"]*)\"")
            .arg(QRegularExpression::escape(key)));
    for (const QString &line : lines)
    {
        const auto match = expression.match(line);
        if (match.hasMatch())
            return match.captured(1);
    }

    return {};
}

bool replaceLocalValue(QStringList &lines, const QString &key, const QString &value)
{
    const QRegularExpression expression(
        QStringLiteral("^(\\s*local\\s+%1\\s*=\\s*)(\"[^\"]*\")")
            .arg(QRegularExpression::escape(key)));
    QString escaped = value;
    escaped.replace(QChar(92), QString(2, QChar(92)));
    escaped.replace(QChar(34), QString(QChar(92)) + QChar(34));

    for (int line = 0; line < lines.size(); ++line)
    {
        const auto match = expression.match(lines.at(line));
        if (!match.hasMatch())
            continue;

        lines[line].replace(match.capturedStart(2), match.capturedLength(2),
                            QStringLiteral("\"%1\"").arg(escaped));
        return true;
    }

    return false;
}

QString boolValue(bool value)
{
    return value ? QStringLiteral("true") : QStringLiteral("false");
}

bool parsedBool(const QString &value, bool fallback)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QLatin1String("true"))
        return true;
    if (normalized == QLatin1String("false"))
        return false;
    return fallback;
}

int integerValue(const QString &value, int fallback, int minimum, int maximum)
{
    bool ok = false;
    const int parsed = value.toInt(&ok);
    return ok ? qBound(minimum, parsed, maximum) : fallback;
}

int percentValue(const QString &value, int fallback)
{
    bool ok = false;
    const double parsed = value.toDouble(&ok);
    return ok ? qBound(10, qRound(parsed * 100.0), 100) : fallback;
}
QString normalizedGradientColor(const QString &value, const QString fallback)
{
    const QString normalized = value.trimmed();
    if (!normalized.startsWith(QStringLiteral("rgba(")))
        return fallback;
    if (!normalized.endsWith(QChar(41)))
        return fallback;
    return normalized.size() == 14 ? normalized : fallback;
}

QString borderGradientColor(const QStringList lines, const TableRange table, int index)
{
    if (table.start < 0 || table.end < table.start)
        return {};

    for (int line = table.start; line <= table.end; ++line)
    {
        const QString source = lines.at(line);
        const int colorsStart = source.indexOf(QStringLiteral("colors"));
        if (colorsStart < 0)
            continue;
        const int firstQuote = source.indexOf(QChar(34), colorsStart);
        const int secondQuote = source.indexOf(QChar(34), firstQuote + 1);
        const int thirdQuote = source.indexOf(QChar(34), secondQuote + 1);
        const int fourthQuote = source.indexOf(QChar(34), thirdQuote + 1);
        if (firstQuote < 0 || secondQuote < 0 || thirdQuote < 0 || fourthQuote < 0)
            continue;
        return index == 0 ? source.mid(firstQuote + 1, secondQuote - firstQuote - 1) : source.mid(thirdQuote + 1, fourthQuote - thirdQuote - 1);
    }

    return {};
}

bool replaceBorderGradientColors(QStringList *lines, const TableRange table, const QString startColor, const QString endColor)
{
    if (!lines)
        return false;
    if (table.start < 0 || table.end < table.start)
        return false;

    for (int line = table.start; line <= table.end; ++line)
    {
        const QString source = lines->at(line);
        const int colorsStart = source.indexOf(QStringLiteral("colors"));
        if (colorsStart < 0)
            continue;
        const int openingBrace = source.indexOf(QChar(123), colorsStart);
        const int closingBrace = source.indexOf(QChar(125), openingBrace + 1);
        if (openingBrace < 0 || closingBrace < 0)
            continue;

        (*lines)[line].replace(openingBrace, closingBrace - openingBrace + 1,
                               QStringLiteral("{ \"%1\", \"%2\" }").arg(startColor, endColor));
        return true;
    }

    return false;
}
}

HyprlandInfo::HyprlandInfo(QObject *parent)
    : QObject(parent)
    , m_configPath(hyprlandConfigPath())
    , m_available(QFileInfo::exists(m_configPath)
                  && !QStandardPaths::findExecutable(QStringLiteral("hyprland")).isEmpty())
{
    load();
}

QString HyprlandInfo::configPath() const { return m_configPath; }
bool HyprlandInfo::available() const { return m_available; }
int HyprlandInfo::gapsIn() const { return m_gapsIn; }
int HyprlandInfo::gapsOut() const { return m_gapsOut; }
int HyprlandInfo::borderSize() const { return m_borderSize; }
QString HyprlandInfo::activeBorderColorStart() const { return m_activeBorderColorStart; }
QString HyprlandInfo::activeBorderColorEnd() const { return m_activeBorderColorEnd; }
QString HyprlandInfo::inactiveBorderColor() const { return m_inactiveBorderColor; }
int HyprlandInfo::borderGradientAngle() const { return m_borderGradientAngle; }
int HyprlandInfo::rounding() const { return m_rounding; }
QString HyprlandInfo::layout() const { return m_layout; }
int HyprlandInfo::activeOpacity() const { return m_activeOpacity; }
int HyprlandInfo::inactiveOpacity() const { return m_inactiveOpacity; }
bool HyprlandInfo::blurEnabled() const { return m_blurEnabled; }
int HyprlandInfo::blurSize() const { return m_blurSize; }
int HyprlandInfo::blurPasses() const { return m_blurPasses; }
bool HyprlandInfo::animationsEnabled() const { return m_animationsEnabled; }
QVariantList HyprlandInfo::windowRules() const { return m_windowRules; }
QString HyprlandInfo::terminal() const { return m_terminal; }
QString HyprlandInfo::fileManager() const { return m_fileManager; }
QString HyprlandInfo::menu() const { return m_menu; }
QString HyprlandInfo::lockScreen() const { return m_lockScreen; }
QString HyprlandInfo::webBrowser() const { return m_webBrowser; }
QString HyprlandInfo::keyboardLayout() const { return m_keyboardLayout; }
QString HyprlandInfo::keyboardVariant() const { return m_keyboardVariant; }
QString HyprlandInfo::keyboardModel() const { return m_keyboardModel; }
QString HyprlandInfo::keyboardOptions() const { return m_keyboardOptions; }
QString HyprlandInfo::keyboardRules() const { return m_keyboardRules; }
int HyprlandInfo::followMouse() const { return m_followMouse; }
double HyprlandInfo::pointerSensitivity() const { return m_pointerSensitivity; }
bool HyprlandInfo::naturalScroll() const { return m_naturalScroll; }
bool HyprlandInfo::workspaceSwipeEnabled() const { return m_workspaceSwipeEnabled; }
int HyprlandInfo::workspaceSwipeFingers() const { return m_workspaceSwipeFingers; }
bool HyprlandInfo::workspaceSwipeInvert() const { return m_workspaceSwipeInvert; }
int HyprlandInfo::workspaceSwipeDistance() const { return m_workspaceSwipeDistance; }
bool HyprlandInfo::pinchZoomGestureEnabled() const { return m_pinchZoomGestureEnabled; }
bool HyprlandInfo::moveWindowGestureEnabled() const { return m_moveWindowGestureEnabled; }
int HyprlandInfo::moveWindowGestureFingers() const { return m_moveWindowGestureFingers; }
QVariantList HyprlandInfo::keybinds() const { return m_keybinds; }
QVariantList HyprlandInfo::devices() const { return m_devices; }

void HyprlandInfo::setChanged() { Q_EMIT settingsChanged(); }

void HyprlandInfo::setGapsIn(int value)
{
    value = qBound(0, value, 64);
    if (m_gapsIn == value)
        return;
    m_gapsIn = value;
    setChanged();
}

void HyprlandInfo::setGapsOut(int value)
{
    value = qBound(0, value, 64);
    if (m_gapsOut == value)
        return;
    m_gapsOut = value;
    setChanged();
}

void HyprlandInfo::setBorderSize(int value)
{
    value = qBound(0, value, 16);
    if (m_borderSize == value)
        return;
    m_borderSize = value;
    setChanged();
}

void HyprlandInfo::setActiveBorderColorStart(const QString &value)
{
    const QString normalized = normalizedGradientColor(value, m_activeBorderColorStart);
    if (m_activeBorderColorStart == normalized)
        return;
    m_activeBorderColorStart = normalized;
    setChanged();
}

void HyprlandInfo::setActiveBorderColorEnd(const QString &value)
{
    const QString normalized = normalizedGradientColor(value, m_activeBorderColorEnd);
    if (m_activeBorderColorEnd == normalized)
        return;
    m_activeBorderColorEnd = normalized;
    setChanged();
}

void HyprlandInfo::setInactiveBorderColor(const QString &value)
{
    const QString normalized = normalizedGradientColor(value, m_inactiveBorderColor);
    if (m_inactiveBorderColor == normalized)
        return;
    m_inactiveBorderColor = normalized;
    setChanged();
}

void HyprlandInfo::setBorderGradientAngle(int value)
{
    value = qBound(0, value, 360);
    if (m_borderGradientAngle == value)
        return;
    m_borderGradientAngle = value;
    setChanged();
}

void HyprlandInfo::setRounding(int value)
{
    value = qBound(0, value, 64);
    if (m_rounding == value)
        return;
    m_rounding = value;
    setChanged();
}

void HyprlandInfo::setLayout(const QString &value)
{
    const QString normalized = value.trimmed().toLower() == QLatin1String("master")
        ? QStringLiteral("master") : QStringLiteral("dwindle");
    if (m_layout == normalized)
        return;
    m_layout = normalized;
    setChanged();
}

void HyprlandInfo::setActiveOpacity(int value)
{
    value = qBound(10, value, 100);
    if (m_activeOpacity == value)
        return;
    m_activeOpacity = value;
    setChanged();
}

void HyprlandInfo::setInactiveOpacity(int value)
{
    value = qBound(10, value, 100);
    if (m_inactiveOpacity == value)
        return;
    m_inactiveOpacity = value;
    setChanged();
}

void HyprlandInfo::setBlurEnabled(bool value)
{
    if (m_blurEnabled == value)
        return;
    m_blurEnabled = value;
    setChanged();
}

void HyprlandInfo::setBlurSize(int value)
{
    value = qBound(1, value, 20);
    if (m_blurSize == value)
        return;
    m_blurSize = value;
    setChanged();
}

void HyprlandInfo::setBlurPasses(int value)
{
    value = qBound(1, value, 10);
    if (m_blurPasses == value)
        return;
    m_blurPasses = value;
    setChanged();
}

void HyprlandInfo::setAnimationsEnabled(bool value)
{
    if (m_animationsEnabled == value)
        return;
    m_animationsEnabled = value;
    setChanged();
}

bool HyprlandInfo::writeLines(const QStringList &lines)
{
    QSaveFile destination(m_configPath);
    if (!destination.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream stream(&destination);
    stream << lines.join(QLatin1Char(10));
    stream.flush();
    if (!destination.commit())
        return false;

    const QString hyprctl = QStandardPaths::findExecutable(QStringLiteral("hyprctl"));
    if (!hyprctl.isEmpty())
        QProcess::startDetached(hyprctl, {QStringLiteral("reload")});
    return true;
}

bool HyprlandInfo::addWindowRule(const QString &type, const QString &name,
                                 const QString &matchKey, const QString &matchValue,
                                 const QString &action)
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QString normalizedType = type == QLatin1String("layer")
        ? QStringLiteral("layer") : QStringLiteral("window");
    const QString normalizedName = name.trimmed();
    const QString normalizedMatchValue = matchValue.trimmed();
    if (normalizedName.isEmpty() || normalizedMatchValue.isEmpty())
        return false;

    QString normalizedMatchKey = matchKey.trimmed().toLower();
    if (normalizedType == QLatin1String("layer"))
        normalizedMatchKey = QStringLiteral("namespace");
    else if (normalizedMatchKey != QLatin1String("class")
             && normalizedMatchKey != QLatin1String("title")
             && normalizedMatchKey != QLatin1String("appid"))
        normalizedMatchKey = QStringLiteral("class");

    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char(10));
    const QStringList updated = HyprlandRules::append(lines, normalizedType, normalizedName,
                                                       normalizedMatchKey, normalizedMatchValue,
                                                       action.trimmed());
    if (!writeLines(updated))
        return false;

    load();
    return true;
}

bool HyprlandInfo::updateWindowRule(int index, const QString &name,
                                   const QString &matchKey, const QString &matchValue,
                                   const QString &action)
{
    if (index < 0 || index >= m_windowRules.size() || name.trimmed().isEmpty()
        || matchValue.trimmed().isEmpty())
        return false;

    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char(10));
    const QStringList updated = HyprlandRules::update(lines, index, name.trimmed(),
                                                       matchKey.trimmed(), matchValue.trimmed(),
                                                       action.trimmed());
    if (updated.isEmpty())
        return false;
    if (!writeLines(updated))
        return false;

    load();
    return true;
}

bool HyprlandInfo::removeWindowRule(int index)
{
    if (index < 0 || index >= m_windowRules.size())
        return false;

    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char(10));
    const QStringList updated = HyprlandRules::remove(lines, index);
    if (!writeLines(updated))
        return false;

    load();
    return true;
}

bool HyprlandInfo::addKeybind(const QString &key, const QString &command)
{
    if (key.trimmed().isEmpty() || command.trimmed().isEmpty()) return false;
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char(10));
    const QStringList updated = HyprlandKeybinds::append(lines, key, command);
    if (!writeLines(updated)) return false;
    load();
    return true;
}

bool HyprlandInfo::updateKeybind(int index, const QString &key, const QString &command, const QString &options, const QString &keyExpression, const QString &actionExpression)
{
    if (index < 0 || index >= m_keybinds.size() || key.trimmed().isEmpty() || command.trimmed().isEmpty()) return false;
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char(10));
    const QStringList updated = HyprlandKeybinds::update(lines, index, key, command, options, keyExpression, actionExpression);
    if (updated.isEmpty() || !writeLines(updated)) return false;
    load();
    return true;
}

bool HyprlandInfo::removeKeybind(int index)
{
    if (index < 0 || index >= m_keybinds.size()) return false;
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char(10));
    const QStringList updated = HyprlandKeybinds::remove(lines, index);
    if (updated.isEmpty() || !writeLines(updated)) return false;
    load();
    return true;
}

bool HyprlandInfo::addDevice(const QString &name, double sensitivity)
{
    if (name.trimmed().isEmpty()) return false;
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char(10));
    const QStringList updated = HyprlandDevices::append(lines, name, qBound(-1.0, sensitivity, 1.0));
    if (!writeLines(updated)) return false;
    load();
    return true;
}

bool HyprlandInfo::updateDevice(int index, const QString &name, double sensitivity)
{
    if (index < 0 || index >= m_devices.size() || name.trimmed().isEmpty()) return false;
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char(10));
    const QStringList updated = HyprlandDevices::update(lines, index, name, qBound(-1.0, sensitivity, 1.0));
    if (updated.isEmpty() || !writeLines(updated)) return false;
    load();
    return true;
}

bool HyprlandInfo::removeDevice(int index)
{
    if (index < 0 || index >= m_devices.size()) return false;
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char(10));
    const QStringList updated = HyprlandDevices::remove(lines, index);
    if (updated.isEmpty() || !writeLines(updated)) return false;
    load();
    return true;
}

void HyprlandInfo::setTerminal(const QString &value)
{
    const QString normalized = value.trimmed();
    if (normalized.isEmpty() || m_terminal == normalized)
        return;
    m_terminal = normalized;
    setChanged();
}

void HyprlandInfo::setFileManager(const QString &value)
{
    const QString normalized = value.trimmed();
    if (normalized.isEmpty() || m_fileManager == normalized)
        return;
    m_fileManager = normalized;
    setChanged();
}

void HyprlandInfo::setMenu(const QString &value)
{
    const QString normalized = value.trimmed();
    if (normalized.isEmpty() || m_menu == normalized)
        return;
    m_menu = normalized;
    setChanged();
}

void HyprlandInfo::setLockScreen(const QString &value)
{
    const QString normalized = value.trimmed();
    if (normalized.isEmpty() || m_lockScreen == normalized)
        return;
    m_lockScreen = normalized;
    setChanged();
}

void HyprlandInfo::setWebBrowser(const QString &value)
{
    const QString normalized = value.trimmed();
    if (normalized.isEmpty() || m_webBrowser == normalized)
        return;
    m_webBrowser = normalized;
    setChanged();
}

void HyprlandInfo::setKeyboardLayout(const QString &value)
{
    const QString normalized = value.trimmed();
    if (normalized.isEmpty() || m_keyboardLayout == normalized) return;
    m_keyboardLayout = normalized;
    setChanged();
}

void HyprlandInfo::setKeyboardVariant(const QString &value)
{
    if (m_keyboardVariant == value) return;
    m_keyboardVariant = value;
    setChanged();
}

void HyprlandInfo::setKeyboardModel(const QString &value)
{
    if (m_keyboardModel == value) return;
    m_keyboardModel = value;
    setChanged();
}

void HyprlandInfo::setKeyboardOptions(const QString &value)
{
    if (m_keyboardOptions == value) return;
    m_keyboardOptions = value;
    setChanged();
}

void HyprlandInfo::setKeyboardRules(const QString &value)
{
    if (m_keyboardRules == value) return;
    m_keyboardRules = value;
    setChanged();
}

void HyprlandInfo::setFollowMouse(int value)
{
    const int normalized = qBound(0, value, 3);
    if (m_followMouse == normalized) return;
    m_followMouse = normalized;
    setChanged();
}

void HyprlandInfo::setPointerSensitivity(double value)
{
    const double normalized = qBound(-1.0, value, 1.0);
    if (qFuzzyCompare(m_pointerSensitivity, normalized)) return;
    m_pointerSensitivity = normalized;
    setChanged();
}

void HyprlandInfo::setNaturalScroll(bool value)
{
    if (m_naturalScroll == value) return;
    m_naturalScroll = value;
    setChanged();
}

void HyprlandInfo::setWorkspaceSwipeEnabled(bool value)
{
    if (m_workspaceSwipeEnabled == value)
        return;
    m_workspaceSwipeEnabled = value;
    setChanged();
}

void HyprlandInfo::setWorkspaceSwipeFingers(int value)
{
    const int normalized = qBound(3, value, 9);
    if (m_workspaceSwipeFingers == normalized)
        return;
    m_workspaceSwipeFingers = normalized;
    setChanged();
}

void HyprlandInfo::setWorkspaceSwipeInvert(bool value)
{
    if (m_workspaceSwipeInvert == value)
        return;
    m_workspaceSwipeInvert = value;
    setChanged();
}

void HyprlandInfo::setWorkspaceSwipeDistance(int value)
{
    const int normalized = qBound(0, value, 2000);
    if (m_workspaceSwipeDistance == normalized)
        return;
    m_workspaceSwipeDistance = normalized;
    setChanged();
}

void HyprlandInfo::setPinchZoomGestureEnabled(bool value)
{
    if (m_pinchZoomGestureEnabled == value)
        return;
    m_pinchZoomGestureEnabled = value;
    setChanged();
}

void HyprlandInfo::setMoveWindowGestureEnabled(bool value)
{
    if (m_moveWindowGestureEnabled == value)
        return;
    m_moveWindowGestureEnabled = value;
    setChanged();
}

void HyprlandInfo::setMoveWindowGestureFingers(int value)
{
    const int normalized = qBound(3, value, 5);
    if (m_moveWindowGestureFingers == normalized)
        return;
    m_moveWindowGestureFingers = normalized;
    setChanged();
}

void HyprlandInfo::reload()
{
    load();
}

void HyprlandInfo::load()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
    const TableRange general = findTable(lines, QStringLiteral("general"));
    const TableRange generalColors = findTable(lines, QStringLiteral("col"), general.start, general.end);
    const TableRange activeBorder = findTable(lines, QStringLiteral("active_border"), generalColors.start, generalColors.end);
    const TableRange decoration = findTable(lines, QStringLiteral("decoration"));
    const TableRange blur = findTable(lines, QStringLiteral("blur"), decoration.start, decoration.end);
    const TableRange animations = findTable(lines, QStringLiteral("animations"));
    const TableRange input = findTable(lines, QStringLiteral("input"));
    const TableRange touchpad = findTable(lines, QStringLiteral("touchpad"), input.start, input.end);
    const TableRange gestures = findTable(lines, QStringLiteral("gestures"));
    const TableRange workspaceSwipeGesture = findWorkspaceSwipeGesture(lines);
    const TableRange pinchZoomGesture = findGesture(lines, QStringLiteral("pinch"), QStringLiteral("cursorzoom"));
    const TableRange moveWindowGesture = findGesture(lines, QStringLiteral("swipe"), QStringLiteral("move"));

    m_gapsIn = integerValue(valueInTable(lines, general, QStringLiteral("gaps_in")), 4, 0, 64);
    m_gapsOut = integerValue(valueInTable(lines, general, QStringLiteral("gaps_out")), 8, 0, 64);
    m_borderSize = integerValue(valueInTable(lines, general, QStringLiteral("border_size")), 1, 0, 16);
    m_activeBorderColorStart = normalizedGradientColor(borderGradientColor(lines, activeBorder, 0), QStringLiteral("rgba(33ccffee)"));
    m_activeBorderColorEnd = normalizedGradientColor(borderGradientColor(lines, activeBorder, 1), QStringLiteral("rgba(00ff99ee)"));
    m_inactiveBorderColor = normalizedGradientColor(valueInTable(lines, generalColors, QStringLiteral("inactive_border")), QStringLiteral("rgba(595959aa)"));
    m_borderGradientAngle = integerValue(valueInTable(lines, activeBorder, QStringLiteral("angle")), 45, 0, 360);
    m_rounding = integerValue(valueInTable(lines, decoration, QStringLiteral("rounding")), 16, 0, 64);
    m_layout = valueInTable(lines, general, QStringLiteral("layout")).trimmed().toLower();
    if (m_layout != QLatin1String("master"))
        m_layout = QStringLiteral("dwindle");
    m_activeOpacity = percentValue(valueInTable(lines, decoration, QStringLiteral("active_opacity")), 100);
    m_inactiveOpacity = percentValue(valueInTable(lines, decoration, QStringLiteral("inactive_opacity")), 80);
    m_blurEnabled = valueInTable(lines, blur, QStringLiteral("enabled")).trimmed().toLower() != QLatin1String("false");
    m_blurSize = integerValue(valueInTable(lines, blur, QStringLiteral("size")), 6, 1, 20);
    m_blurPasses = integerValue(valueInTable(lines, blur, QStringLiteral("passes")), 3, 1, 10);
    m_animationsEnabled = valueInTable(lines, animations, QStringLiteral("enabled")).trimmed().toLower() != QLatin1String("false");
    m_windowRules = HyprlandRules::parse(lines);
    m_keyboardLayout = valueInTable(lines, input, QStringLiteral("kb_layout"));
    if (m_keyboardLayout.isEmpty()) m_keyboardLayout = QStringLiteral("us");
    m_keyboardVariant = valueInTable(lines, input, QStringLiteral("kb_variant"));
    m_keyboardModel = valueInTable(lines, input, QStringLiteral("kb_model"));
    m_keyboardOptions = valueInTable(lines, input, QStringLiteral("kb_options"));
    m_keyboardRules = valueInTable(lines, input, QStringLiteral("kb_rules"));
    m_followMouse = integerValue(valueInTable(lines, input, QStringLiteral("follow_mouse")), 1, 0, 3);
    bool sensitivityOk = false;
    const double parsedSensitivity = valueInTable(lines, input, QStringLiteral("sensitivity")).toDouble(&sensitivityOk);
    m_pointerSensitivity = sensitivityOk ? qBound(-1.0, parsedSensitivity, 1.0) : 0.0;
    m_naturalScroll = valueInTable(lines, touchpad, QStringLiteral("natural_scroll")).trimmed().toLower() == QLatin1String("true");
    m_workspaceSwipeEnabled = workspaceSwipeGesture.start >= 0;
    m_workspaceSwipeFingers = integerValue(valueInTable(lines, workspaceSwipeGesture, QStringLiteral("fingers")), 3, 3, 9);
    m_workspaceSwipeInvert = parsedBool(valueInTable(lines, gestures, QStringLiteral("workspace_swipe_invert")), true);
    m_workspaceSwipeDistance = integerValue(valueInTable(lines, gestures, QStringLiteral("workspace_swipe_distance")), 300, 0, 2000);
    m_pinchZoomGestureEnabled = pinchZoomGesture.start >= 0;
    m_moveWindowGestureEnabled = moveWindowGesture.start >= 0;
    m_moveWindowGestureFingers = integerValue(valueInTable(lines, moveWindowGesture, QStringLiteral("fingers")), 4, 3, 5);
    m_keybinds = HyprlandKeybinds::parse(lines);
    m_devices = HyprlandDevices::parse(lines);
    m_terminal = localValue(lines, QStringLiteral("terminal"));
    if (m_terminal.isEmpty())
        m_terminal = QStringLiteral("station");
    m_fileManager = localValue(lines, QStringLiteral("fileManager"));
    if (m_fileManager.isEmpty())
        m_fileManager = QStringLiteral("index");
    m_menu = localValue(lines, QStringLiteral("menu"));
    if (m_menu.isEmpty())
        m_menu = QStringLiteral("vicinae toggle");
    m_lockScreen = localValue(lines, QStringLiteral("lockScreen"));
    if (m_lockScreen.isEmpty())
        m_lockScreen = QStringLiteral("desklock");
    m_webBrowser = localValue(lines, QStringLiteral("webBrowser"));
    if (m_webBrowser.isEmpty())
        m_webBrowser = QStringLiteral("fiery");


    setChanged();
}

bool HyprlandInfo::save()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
    const TableRange general = findTable(lines, QStringLiteral("general"));
    const TableRange generalColors = findTable(lines, QStringLiteral("col"), general.start, general.end);
    const TableRange activeBorder = findTable(lines, QStringLiteral("active_border"), generalColors.start, generalColors.end);
    const TableRange decoration = findTable(lines, QStringLiteral("decoration"));
    const TableRange blur = findTable(lines, QStringLiteral("blur"), decoration.start, decoration.end);
    const TableRange animations = findTable(lines, QStringLiteral("animations"));
    const TableRange input = findTable(lines, QStringLiteral("input"));
    const TableRange touchpad = findTable(lines, QStringLiteral("touchpad"), input.start, input.end);
    TableRange gestures = findTable(lines, QStringLiteral("gestures"));
    TableRange workspaceSwipeGesture = findWorkspaceSwipeGesture(lines);
    TableRange pinchZoomGesture = findGesture(lines, QStringLiteral("pinch"), QStringLiteral("cursorzoom"));
    TableRange moveWindowGesture = findGesture(lines, QStringLiteral("swipe"), QStringLiteral("move"));

    if (gestures.start < 0 && input.end >= input.start)
    {
        int gestureInsertAt = input.end + 1;
        lines.insert(gestureInsertAt++, QString());
        lines.insert(gestureInsertAt++, QStringLiteral("hl.config({"));
        lines.insert(gestureInsertAt++, QStringLiteral("    gestures = {"));
        lines.insert(gestureInsertAt++, QStringLiteral("        workspace_swipe_distance = 300,"));
        lines.insert(gestureInsertAt++, QStringLiteral("        workspace_swipe_invert = true,"));
        lines.insert(gestureInsertAt++, QStringLiteral("    },"));
        lines.insert(gestureInsertAt++, QStringLiteral("})"));
        lines.insert(gestureInsertAt, QString());
        gestures = findTable(lines, QStringLiteral("gestures"));
        workspaceSwipeGesture = findWorkspaceSwipeGesture(lines);
        pinchZoomGesture = findGesture(lines, QStringLiteral("pinch"), QStringLiteral("cursorzoom"));
        moveWindowGesture = findGesture(lines, QStringLiteral("swipe"), QStringLiteral("move"));
    }

    const bool updated = replaceValue(lines, general, QStringLiteral("gaps_in"), QString::number(m_gapsIn))
        && replaceValue(lines, general, QStringLiteral("gaps_out"), QString::number(m_gapsOut))
        && replaceValue(lines, general, QStringLiteral("border_size"), QString::number(m_borderSize))
        && replaceBorderGradientColors(&lines, activeBorder, m_activeBorderColorStart, m_activeBorderColorEnd)
        && replaceValue(lines, generalColors, QStringLiteral("inactive_border"), QStringLiteral("\"%1\"").arg(m_inactiveBorderColor))
        && replaceValue(lines, activeBorder, QStringLiteral("angle"), QString::number(m_borderGradientAngle))
        && replaceValue(lines, general, QStringLiteral("layout"), QStringLiteral("\"%1\"").arg(m_layout))
        && replaceValue(lines, decoration, QStringLiteral("rounding"), QString::number(m_rounding))
        && replaceValue(lines, decoration, QStringLiteral("active_opacity"), QString::number(m_activeOpacity / 100.0, 'f', 2))
        && replaceValue(lines, decoration, QStringLiteral("inactive_opacity"), QString::number(m_inactiveOpacity / 100.0, 'f', 2))
        && replaceValue(lines, blur, QStringLiteral("enabled"), boolValue(m_blurEnabled))
        && replaceValue(lines, blur, QStringLiteral("size"), QString::number(m_blurSize))
        && replaceValue(lines, blur, QStringLiteral("passes"), QString::number(m_blurPasses))
        && replaceValue(lines, animations, QStringLiteral("enabled"), boolValue(m_animationsEnabled))
        && replaceValue(lines, input, QStringLiteral("kb_layout"), QStringLiteral("\"%1\"").arg(m_keyboardLayout))
        && replaceValue(lines, input, QStringLiteral("kb_variant"), QStringLiteral("\"%1\"").arg(m_keyboardVariant))
        && replaceValue(lines, input, QStringLiteral("kb_model"), QStringLiteral("\"%1\"").arg(m_keyboardModel))
        && replaceValue(lines, input, QStringLiteral("kb_options"), QStringLiteral("\"%1\"").arg(m_keyboardOptions))
        && replaceValue(lines, input, QStringLiteral("kb_rules"), QStringLiteral("\"%1\"").arg(m_keyboardRules))
        && replaceValue(lines, input, QStringLiteral("follow_mouse"), QString::number(m_followMouse))
        && replaceValue(lines, input, QStringLiteral("sensitivity"), QString::number(m_pointerSensitivity, 'f', 2))
        && replaceValue(lines, touchpad, QStringLiteral("natural_scroll"), boolValue(m_naturalScroll))
        && replaceValue(lines, gestures, QStringLiteral("workspace_swipe_distance"), QString::number(m_workspaceSwipeDistance))
        && replaceValue(lines, gestures, QStringLiteral("workspace_swipe_invert"), boolValue(m_workspaceSwipeInvert))
        && replaceLocalValue(lines, QStringLiteral("terminal"), m_terminal)
        && replaceLocalValue(lines, QStringLiteral("fileManager"), m_fileManager)
        && replaceLocalValue(lines, QStringLiteral("menu"), m_menu)
        && replaceLocalValue(lines, QStringLiteral("lockScreen"), m_lockScreen)
        && replaceLocalValue(lines, QStringLiteral("webBrowser"), m_webBrowser);
    if (!updated)
        return false;

    if (m_moveWindowGestureEnabled)
    {
        if (moveWindowGesture.start < 0)
        {
            int gestureInsertAt = lines.size();
            lines.insert(gestureInsertAt++, QString());
            lines.insert(gestureInsertAt++, QStringLiteral("hl.gesture({"));
            lines.insert(gestureInsertAt++, QStringLiteral("    fingers = %1,").arg(m_moveWindowGestureFingers));
            lines.insert(gestureInsertAt++, QStringLiteral("    direction = \"swipe\","));
            lines.insert(gestureInsertAt++, QStringLiteral("    action = \"move\","));
            lines.insert(gestureInsertAt++, QStringLiteral("})"));
            lines.insert(gestureInsertAt, QString());
        }
    }
    else if (moveWindowGesture.start >= 0)
    {
        lines.erase(lines.begin() + moveWindowGesture.start,
                    lines.begin() + moveWindowGesture.end + 1);
    }

    pinchZoomGesture = findGesture(lines, QStringLiteral("pinch"), QStringLiteral("cursorzoom"));
    if (m_pinchZoomGestureEnabled)
    {
        if (pinchZoomGesture.start < 0)
        {
            int gestureInsertAt = lines.size();
            lines.insert(gestureInsertAt++, QString());
            lines.insert(gestureInsertAt++, QStringLiteral("hl.gesture({"));
            lines.insert(gestureInsertAt++, QStringLiteral("    fingers = 2,"));
            lines.insert(gestureInsertAt++, QStringLiteral("    direction = \"pinch\","));
            lines.insert(gestureInsertAt++, QStringLiteral("    action = \"cursorZoom\","));
            lines.insert(gestureInsertAt++, QStringLiteral("    zoom_level = \"1\","));
            lines.insert(gestureInsertAt++, QStringLiteral("    mode = \"live\","));
            lines.insert(gestureInsertAt++, QStringLiteral("})"));
            lines.insert(gestureInsertAt, QString());
        }
    }
    else if (pinchZoomGesture.start >= 0)
    {
        lines.erase(lines.begin() + pinchZoomGesture.start,
                    lines.begin() + pinchZoomGesture.end + 1);
    }

    workspaceSwipeGesture = findWorkspaceSwipeGesture(lines);
    if (m_workspaceSwipeEnabled)
    {
        if (workspaceSwipeGesture.start >= 0)
        {
            if (!replaceValue(lines, workspaceSwipeGesture, QStringLiteral("fingers"), QString::number(m_workspaceSwipeFingers)))
                return false;
        }
        else
        {
            int gestureInsertAt = input.end >= input.start ? input.end + 1 : lines.size();
            lines.insert(gestureInsertAt++, QString());
            lines.insert(gestureInsertAt++, QStringLiteral("hl.gesture({"));
            lines.insert(gestureInsertAt++, QStringLiteral("    fingers = %1,").arg(m_workspaceSwipeFingers));
            lines.insert(gestureInsertAt++, QStringLiteral("    direction = \"horizontal\","));
            lines.insert(gestureInsertAt++, QStringLiteral("    action = \"workspace\","));
            lines.insert(gestureInsertAt++, QStringLiteral("})"));
            lines.insert(gestureInsertAt, QString());
        }
    }
    else if (workspaceSwipeGesture.start >= 0)
    {
        lines.erase(lines.begin() + workspaceSwipeGesture.start,
                    lines.begin() + workspaceSwipeGesture.end + 1);
    }

    QSaveFile destination(m_configPath);
    if (!destination.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream stream(&destination);
    stream << lines.join(QLatin1Char('\n'));
    stream.flush();
    if (!destination.commit())
        return false;

    const QString hyprctl = QStandardPaths::findExecutable(QStringLiteral("hyprctl"));
    if (!hyprctl.isEmpty())
        QProcess::startDetached(hyprctl, {QStringLiteral("reload")});
    return true;
}
