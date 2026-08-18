#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class HyprlandInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configPath READ configPath CONSTANT)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(int gapsIn READ gapsIn WRITE setGapsIn NOTIFY settingsChanged)
    Q_PROPERTY(int gapsOut READ gapsOut WRITE setGapsOut NOTIFY settingsChanged)
    Q_PROPERTY(int borderSize READ borderSize WRITE setBorderSize NOTIFY settingsChanged)
    Q_PROPERTY(QString activeBorderColorStart READ activeBorderColorStart WRITE setActiveBorderColorStart NOTIFY settingsChanged)
    Q_PROPERTY(QString activeBorderColorEnd READ activeBorderColorEnd WRITE setActiveBorderColorEnd NOTIFY settingsChanged)
    Q_PROPERTY(QString inactiveBorderColor READ inactiveBorderColor WRITE setInactiveBorderColor NOTIFY settingsChanged)
    Q_PROPERTY(int borderGradientAngle READ borderGradientAngle WRITE setBorderGradientAngle NOTIFY settingsChanged)
    Q_PROPERTY(int rounding READ rounding WRITE setRounding NOTIFY settingsChanged)
    Q_PROPERTY(QString layout READ layout WRITE setLayout NOTIFY settingsChanged)
    Q_PROPERTY(int activeOpacity READ activeOpacity WRITE setActiveOpacity NOTIFY settingsChanged)
    Q_PROPERTY(int inactiveOpacity READ inactiveOpacity WRITE setInactiveOpacity NOTIFY settingsChanged)
    Q_PROPERTY(bool blurEnabled READ blurEnabled WRITE setBlurEnabled NOTIFY settingsChanged)
    Q_PROPERTY(int blurSize READ blurSize WRITE setBlurSize NOTIFY settingsChanged)
    Q_PROPERTY(int blurPasses READ blurPasses WRITE setBlurPasses NOTIFY settingsChanged)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled WRITE setAnimationsEnabled NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList windowRules READ windowRules NOTIFY settingsChanged)
    Q_PROPERTY(QString keyboardLayout READ keyboardLayout WRITE setKeyboardLayout NOTIFY settingsChanged)
    Q_PROPERTY(QString keyboardVariant READ keyboardVariant WRITE setKeyboardVariant NOTIFY settingsChanged)
    Q_PROPERTY(QString keyboardModel READ keyboardModel WRITE setKeyboardModel NOTIFY settingsChanged)
    Q_PROPERTY(QString keyboardOptions READ keyboardOptions WRITE setKeyboardOptions NOTIFY settingsChanged)
    Q_PROPERTY(QString keyboardRules READ keyboardRules WRITE setKeyboardRules NOTIFY settingsChanged)
    Q_PROPERTY(int followMouse READ followMouse WRITE setFollowMouse NOTIFY settingsChanged)
    Q_PROPERTY(double pointerSensitivity READ pointerSensitivity WRITE setPointerSensitivity NOTIFY settingsChanged)
    Q_PROPERTY(bool naturalScroll READ naturalScroll WRITE setNaturalScroll NOTIFY settingsChanged)
    Q_PROPERTY(bool workspaceSwipeEnabled READ workspaceSwipeEnabled WRITE setWorkspaceSwipeEnabled NOTIFY settingsChanged)
    Q_PROPERTY(int workspaceSwipeFingers READ workspaceSwipeFingers WRITE setWorkspaceSwipeFingers NOTIFY settingsChanged)
    Q_PROPERTY(bool workspaceSwipeInvert READ workspaceSwipeInvert WRITE setWorkspaceSwipeInvert NOTIFY settingsChanged)
    Q_PROPERTY(int workspaceSwipeDistance READ workspaceSwipeDistance WRITE setWorkspaceSwipeDistance NOTIFY settingsChanged)
    Q_PROPERTY(bool pinchZoomGestureEnabled READ pinchZoomGestureEnabled WRITE setPinchZoomGestureEnabled NOTIFY settingsChanged)
    Q_PROPERTY(bool moveWindowGestureEnabled READ moveWindowGestureEnabled WRITE setMoveWindowGestureEnabled NOTIFY settingsChanged)
    Q_PROPERTY(int moveWindowGestureFingers READ moveWindowGestureFingers WRITE setMoveWindowGestureFingers NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList keybinds READ keybinds NOTIFY settingsChanged)
    Q_PROPERTY(QVariantList devices READ devices NOTIFY settingsChanged)
    Q_PROPERTY(QString terminal READ terminal WRITE setTerminal NOTIFY settingsChanged)
    Q_PROPERTY(QString fileManager READ fileManager WRITE setFileManager NOTIFY settingsChanged)
    Q_PROPERTY(QString menu READ menu WRITE setMenu NOTIFY settingsChanged)
    Q_PROPERTY(QString lockScreen READ lockScreen WRITE setLockScreen NOTIFY settingsChanged)
    Q_PROPERTY(QString webBrowser READ webBrowser WRITE setWebBrowser NOTIFY settingsChanged)

public:
    explicit HyprlandInfo(QObject *parent = nullptr);

    QString configPath() const;
    bool available() const;
    int gapsIn() const;
    int gapsOut() const;
    int borderSize() const;
    QString activeBorderColorStart() const;
    QString activeBorderColorEnd() const;
    QString inactiveBorderColor() const;
    int borderGradientAngle() const;
    int rounding() const;
    QString layout() const;
    int activeOpacity() const;
    int inactiveOpacity() const;
    bool blurEnabled() const;
    int blurSize() const;
    int blurPasses() const;
    bool animationsEnabled() const;
    QVariantList windowRules() const;
    QString keyboardLayout() const;
    QString keyboardVariant() const;
    QString keyboardModel() const;
    QString keyboardOptions() const;
    QString keyboardRules() const;
    int followMouse() const;
    double pointerSensitivity() const;
    bool naturalScroll() const;
    bool workspaceSwipeEnabled() const;
    int workspaceSwipeFingers() const;
    bool workspaceSwipeInvert() const;
    int workspaceSwipeDistance() const;
    bool pinchZoomGestureEnabled() const;
    bool moveWindowGestureEnabled() const;
    int moveWindowGestureFingers() const;
    QVariantList keybinds() const;
    QVariantList devices() const;
    QString terminal() const;
    QString fileManager() const;
    QString menu() const;
    QString lockScreen() const;
    QString webBrowser() const;

    void setGapsIn(int value);
    void setGapsOut(int value);
    void setBorderSize(int value);
    void setActiveBorderColorStart(const QString &value);
    void setActiveBorderColorEnd(const QString &value);
    void setInactiveBorderColor(const QString &value);
    void setBorderGradientAngle(int value);
    void setRounding(int value);
    void setLayout(const QString &value);
    void setActiveOpacity(int value);
    void setInactiveOpacity(int value);
    void setBlurEnabled(bool value);
    void setBlurSize(int value);
    void setBlurPasses(int value);
    void setAnimationsEnabled(bool value);
    Q_INVOKABLE bool addWindowRule(const QString &type, const QString &name, const QString &matchKey, const QString &matchValue, const QString &action);
    Q_INVOKABLE bool updateWindowRule(int index, const QString &name, const QString &matchKey, const QString &matchValue, const QString &action);
    Q_INVOKABLE bool removeWindowRule(int index);
    Q_INVOKABLE bool addKeybind(const QString &key, const QString &command);
    Q_INVOKABLE bool updateKeybind(int index, const QString &key, const QString &command, const QString &options = {}, const QString &keyExpression = {}, const QString &actionExpression = {});
    Q_INVOKABLE bool removeKeybind(int index);
    Q_INVOKABLE bool addDevice(const QString &name, double sensitivity);
    Q_INVOKABLE bool updateDevice(int index, const QString &name, double sensitivity);
    Q_INVOKABLE bool removeDevice(int index);
    void setKeyboardLayout(const QString &value);
    void setKeyboardVariant(const QString &value);
    void setKeyboardModel(const QString &value);
    void setKeyboardOptions(const QString &value);
    void setKeyboardRules(const QString &value);
    void setFollowMouse(int value);
    void setPointerSensitivity(double value);
    void setNaturalScroll(bool value);
    void setWorkspaceSwipeEnabled(bool value);
    void setWorkspaceSwipeFingers(int value);
    void setWorkspaceSwipeInvert(bool value);
    void setWorkspaceSwipeDistance(int value);
    void setPinchZoomGestureEnabled(bool value);
    void setMoveWindowGestureEnabled(bool value);
    void setMoveWindowGestureFingers(int value);
    void setTerminal(const QString &value);
    void setFileManager(const QString &value);
    void setMenu(const QString &value);
    void setLockScreen(const QString &value);
    void setWebBrowser(const QString &value);

    Q_INVOKABLE void reload();
    Q_INVOKABLE bool save();

Q_SIGNALS:
    void settingsChanged();

private:
    void load();
    void setChanged();
    bool writeLines(const QStringList &lines);

    QString m_configPath;
    bool m_available = false;
    int m_gapsIn = 4;
    int m_gapsOut = 8;
    int m_borderSize = 1;
    QString m_activeBorderColorStart = QStringLiteral("rgba(33ccffee)");
    QString m_activeBorderColorEnd = QStringLiteral("rgba(00ff99ee)");
    QString m_inactiveBorderColor = QStringLiteral("rgba(595959aa)");
    int m_borderGradientAngle = 45;
    int m_rounding = 16;
    QString m_layout = QStringLiteral("dwindle");
    int m_activeOpacity = 100;
    int m_inactiveOpacity = 80;
    bool m_blurEnabled = true;
    int m_blurSize = 6;
    int m_blurPasses = 3;
    bool m_animationsEnabled = true;
    QVariantList m_windowRules;
    QString m_keyboardLayout = QStringLiteral("us");
    QString m_keyboardVariant;
    QString m_keyboardModel;
    QString m_keyboardOptions;
    QString m_keyboardRules;
    int m_followMouse = 1;
    double m_pointerSensitivity = 0.0;
    bool m_naturalScroll = false;
    bool m_workspaceSwipeEnabled = false;
    int m_workspaceSwipeFingers = 3;
    bool m_workspaceSwipeInvert = true;
    int m_workspaceSwipeDistance = 300;
    bool m_pinchZoomGestureEnabled = false;
    bool m_moveWindowGestureEnabled = false;
    int m_moveWindowGestureFingers = 4;
    QVariantList m_keybinds;
    QVariantList m_devices;
    QString m_terminal = QStringLiteral("station");
    QString m_fileManager = QStringLiteral("index");
    QString m_menu = QStringLiteral("vicinae toggle");
    QString m_lockScreen = QStringLiteral("desklock");
    QString m_webBrowser = QStringLiteral("fiery");
};
