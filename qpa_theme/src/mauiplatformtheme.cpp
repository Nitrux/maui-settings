/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Kevin Ottens <ervin+bluesystems@kde.org>
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2014 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/


#include "mauiplatformtheme.h"

#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QFont>
#include <QPalette>
#include <QStyle>
#include <QString>
#include <QVariant>
#include <QtQuickControls2/QQuickStyle>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>
#include <QToolBar>
#include <QMainWindow>
#include <QWindow>
#include <QStyleFactory>


#include <KLocalizedString>
#include <KStandardGuiItem>
#include <KColorScheme>

#include <KIconEngine>
#include <KIconLoader>
#include <kstandardshortcut.h>

#include <KFileItem>
#include <KIO/Global>

#include <private/qiconloader_p.h>

#include <MauiMan4/thememanager.h>
#include <MauiMan4/accessibilitymanager.h>

static const QByteArray s_x11AppMenuServiceNamePropertyName = QByteArrayLiteral("_KDE_NET_WM_APPMENU_SERVICE_NAME");
static const QByteArray s_x11AppMenuObjectPathPropertyName = QByteArrayLiteral("_KDE_NET_WM_APPMENU_OBJECT_PATH");

static bool checkDBusGlobalMenuAvailable()
{
    if (qEnvironmentVariableIsSet("KDE_NO_GLOBAL_MENU")) {
        return false;
    }

    QDBusConnection connection = QDBusConnection::sessionBus();
    QString registrarService = QStringLiteral("com.canonical.AppMenu.Registrar");
    return connection.interface()->isServiceRegistered(registrarService);
}


static QString desktopPortalService()
{
    return QStringLiteral("org.freedesktop.impl.portal.desktop.kde");
}

static QString desktopPortalPath()
{
    return QStringLiteral("/org/freedesktop/portal/desktop");
}

MauiPlatformTheme::MauiPlatformTheme(QObject *parent ) : QObject(parent)
  ,m_theme(new MauiMan::ThemeManager(this))
  ,m_accessibility(new MauiMan::AccessibilityManager(this))
{


    m_defaultFont = new QFont{"Noto Sans", 10, QFont::Normal};
    m_defaultFont->setStyleHint(QFont::SansSerif);

    m_smallFont = new QFont{"Noto Sans", 8, QFont::Normal};
    m_smallFont->setStyleHint(QFont::SansSerif);

    m_monospaceFont = new QFont{"Hack", 8, QFont::Normal};
    m_monospaceFont->setStyleHint(QFont::Monospace);


    loadSettings();

    // explicitly not KWindowSystem::isPlatformWayland to not include the kwin process
    //    if (QGuiApplication::platformName() == QLatin1String("wayland")) {
    //        m_kwaylandIntegration.reset(new KWaylandIntegration(this));
    //    }


    // Don't show the titlebar "What's This?" help button for dialogs that
    // have any UI elements with help text, unless the window specifically
    // requests it. This is because KDE apps with help text will use the nice
    // contextual tooltips instead; we only want to ever see the titlebar button
    // to invoke the "What's This?" feature in 3rd-party Qt apps that have set
    // "What's This" help text and requested the button
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton, true);
#endif

    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, false);
    setQtQuickControlsTheme();

    //    QMetaObject::invokeMethod(this, "setupIconLoader", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "applySettings", Qt::QueuedConnection);

    connect(m_accessibility, &MauiMan::AccessibilityManager::singleClickChanged, [this](bool value)
    {
        m_singleClick = value;
    });

    connect(m_theme, &MauiMan::ThemeManager::iconThemeChanged, [this](const QString &value)
    {
        m_iconTheme = value;

        QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance());
        if (!app)
        {
            return;
        }

        QIcon::setThemeName(m_iconTheme);

       
        // update all widgets for repaint new themed icons.
        for (auto widget : QApplication::allWidgets())
        {
            //this is taken from ukui imp. Supossedly request all widgets to be repainted
            widget->update();

            //this is taken from kde platform theme integration. it only sends signal event to some widgets
            if (qobject_cast<QToolBar *>(widget) || qobject_cast<QMainWindow *>(widget)) {
                QEvent event(QEvent::StyleChange);
                QApplication::sendEvent(widget, &event);
            }
        }

        //this is for now what maui qqc2 theme checks for updating icons
        app->setStyle(m_style);
        
        QEvent e{QEvent::ThemeChange};
        QApplication::sendEvent(this, &e);
        
        QWindowSystemInterface::handleThemeChange();
    });


    connect(m_theme, &MauiMan::ThemeManager::iconSizeChanged, [this](uint value)
    {
        m_iconSize = value;
        iconChanged();
    });

    connect(m_theme, &MauiMan::ThemeManager::defaultFontChanged, [this](QString font)
    {
        if(!m_defaultFont->fromString(font))
        {
            qWarning() << "Failed to set the default font" << font;
            return;
        }

        if (qobject_cast<QApplication *>(QCoreApplication::instance()))
        {
            QApplication::setFont(*m_defaultFont);
            QWidgetList widgets = QApplication::allWidgets();
            for (QWidget *widget : widgets)
            {
                widget->setFont(*m_defaultFont);
            }
        } else
        {
            QGuiApplication::setFont(*m_defaultFont);
        }

    });

    connect(m_theme, &MauiMan::ThemeManager::monospacedFontChanged, [this](QString font)
    {
        if(!m_monospaceFont->fromString(font))
        {
            qWarning() << "Failed to set the default font" << font;
            return;
        }

        QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance());
        if (!app)
        {
            return;
        }

        //this is for now what maui qqc2 theme checks for updating the monospaced fonts, since there is not a qt signal to check for
        qApp->setStyle(m_style);

    });

    connect(m_theme, &MauiMan::ThemeManager::styleTypeChanged, [this](uint type)
    {
                qDebug() << "Style type changed in Maui QPA" << type;

        if(!m_usePalette)
            return;

        bool isDarkPreferred = type == 1;
        auto preferredScheme = type == 3 ? m_theme->customColorScheme() : (isDarkPreferred ? "NitruxDark" : "Nitrux");

        m_palette = new QPalette(loadColorScheme(preferredScheme, m_schemePath));

        qDebug() << "Setting color scheme" << preferredScheme << m_schemePath;

        if(hasWidgets())
        {
            qApp->setPalette(*m_palette);
            qApp->setProperty("KDE_COLOR_SCHEME_PATH", m_schemePath);
        }
        
         QWindowSystemInterface::handleThemeChange();
   
    });

    connect(m_theme, &MauiMan::ThemeManager::customColorSchemeChanged, [this](const QString &scheme)
    {
        if(!m_usePalette)
            return;

        if(m_theme->styleType() != 3)
            return;

        m_palette = new QPalette(loadColorScheme(scheme, m_schemePath));

        qDebug() << "Setting color scheme" << scheme << m_schemePath;

        if(hasWidgets())
        {
            qApp->setProperty("KDE_COLOR_SCHEME_PATH", m_schemePath);
            qApp->setPalette(*m_palette);
        }

//        QGuiApplication::setPalette(*m_palette);

    });
}

void MauiPlatformTheme::setupIconLoader()
{
    //    QObject::connect(KIconLoader::global(), &KIconLoader::iconChanged, this, &MauiPlatformTheme::iconChanged);
}

void MauiPlatformTheme::iconChanged()
{
    //    KIconLoader::Group iconGroup = (KIconLoader::Group)group;
    //    if (iconGroup != KIconLoader::MainToolbar)
    //    {
    //        //        m_hints[QPlatformTheme::SystemIconThemeName] = readConfigValue(QStringLiteral("Icons"), QStringLiteral("Theme"), QStringLiteral("breeze"));
    //        return;
    //    }

    //    const int currentSize = KIconLoader::global()->currentSize(KIconLoader::MainToolbar);
    //    if (m_iconSize == currentSize) {
    //        return;
    //    }

    //    m_iconSize = currentSize;

    // If we are not a QApplication, means that we are a QGuiApplication, then we do nothing.
    if (!qobject_cast<QApplication *>(QCoreApplication::instance())) {
        return;
    }

    const QWidgetList widgets = QApplication::allWidgets();
    for (QWidget *widget : widgets) {
        //        if (qobject_cast<QToolBar *>(widget) || qobject_cast<QMainWindow *>(widget)) {
        QEvent event(QEvent::StyleChange);
        QApplication::sendEvent(widget, &event);
        //        }
    }
}



MauiPlatformTheme::~MauiPlatformTheme()
{
    delete m_defaultFont;
    delete m_smallFont;
    delete m_monospaceFont;
    delete m_palette;
}

static QStringList xdgIconThemePaths()
{
    QStringList paths;

    // make sure we have ~/.local/share/icons in paths if it exists
    paths << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("icons"), QStandardPaths::LocateDirectory);

    const QFileInfo homeIconDir(QDir::homePath() + QStringLiteral("/.icons"));
    if (homeIconDir.isDir()) {
        paths << homeIconDir.absoluteFilePath();
    }

    return paths;
}

#ifdef QT_WIDGETS_LIB
bool MauiPlatformTheme::hasWidgets()
{
    return qobject_cast<QApplication *> (qApp) != nullptr;
}
#endif

QVariant MauiPlatformTheme::themeHint(QPlatformTheme::ThemeHint hintType) const
{
    switch (hintType)
    {
    case QPlatformTheme::CursorFlashTime:
        return m_cursorFlashTime;
    case MouseDoubleClickInterval:
        return m_doubleClickInterval;
    case QPlatformTheme::ToolButtonStyle:
        return m_toolButtonStyle;
    case QPlatformTheme::SystemIconThemeName:
        return m_iconTheme;
    case QPlatformTheme::StyleNames:
        return  QStringList {
            QStringLiteral("breeze"),
                    QStringLiteral("oxygen"),
                    QStringLiteral("fusion"),
                    QStringLiteral("windows"),
        };
    case QPlatformTheme::IconThemeSearchPaths:
        return xdgIconThemePaths();
    case QPlatformTheme::DialogButtonBoxLayout:
        return m_buttonBoxLayout;
    case QPlatformTheme::ToolBarIconSize:
        return m_iconSize;
    case QPlatformTheme::SystemIconFallbackThemeName:
        return "breeze";
    case QPlatformTheme::ItemViewActivateItemOnSingleClick:
        return m_singleClick;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    case QPlatformTheme::KeyboardScheme:
        return m_keyboardScheme;
#endif
    case QPlatformTheme::UiEffects:
        return m_uiEffects;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    case QPlatformTheme::WheelScrollLines:
        return m_wheelScrollLines;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    case QPlatformTheme::ShowShortcutsInContextMenus:
        return m_showShortcutsInContextMenus;
#endif
    default:
        return QPlatformTheme::themeHint(hintType);
    }
}

void MauiPlatformTheme::applySettings()
{
    if(!QGuiApplication::desktopSettingsAware())
        return;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    //do not override application palette
    if(QCoreApplication::testAttribute(Qt::AA_SetPalette))
    {
        m_usePalette = false;
        qDebug() << "palette support is disabled";
    }
#endif

    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, !m_showIconsInMenus);

#ifdef QT_WIDGETS_LIB
    if(hasWidgets())
    {
        qApp->setFont(*m_defaultFont);

        //Qt 5.6 or higher should be use themeHint function on application startup.
        //So, there is no need to call this function first time. ??
        qApp->setWheelScrollLines(m_wheelScrollLines);
        qApp->setStyle(m_style); //recreate style object

        if(!m_palette)
            m_palette = new QPalette(qApp->style()->standardPalette());

        if(m_usePalette)
        {
            qApp->setPalette(*m_palette);
            qApp->setProperty("KDE_COLOR_SCHEME_PATH", m_schemePath);
        }
    }
#endif
    QGuiApplication::setFont(*m_defaultFont); //apply font

    if(m_palette && m_usePalette)
        QGuiApplication::setPalette(*m_palette); //apply palette
}

QIcon MauiPlatformTheme::fileIcon(const QFileInfo &fileInfo, QPlatformTheme::IconOptions iconOptions) const
{
    if (iconOptions.testFlag(DontUseCustomDirectoryIcons) && fileInfo.isDir()) {
        return QIcon::fromTheme(QLatin1String("inode-directory"));
    }

    return QIcon::fromTheme(KIO::iconNameForUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath())));
}

const QPalette *MauiPlatformTheme::palette(Palette type) const
{

    if (m_palette) {
        return m_palette;
    } else {
        return QPlatformTheme::palette(type);
    }
}

const QFont *MauiPlatformTheme::font(Font type) const
{
    switch (type) {
    case SystemFont:
        return m_defaultFont;
    case MenuFont:
    case MenuBarFont:
    case MenuItemFont:
        return m_defaultFont;
    case MessageBoxFont:
    case LabelFont:
    case TipLabelFont:
    case StatusBarFont:
    case PushButtonFont:
    case ItemViewFont:
    case ListViewFont:
    case HeaderViewFont:
    case ListBoxFont:
    case ComboMenuItemFont:
    case ComboLineEditFont:
        return m_defaultFont;
    case TitleBarFont:
    case MdiSubWindowTitleFont:
    case DockWidgetTitleFont:
        return m_defaultFont;
    case SmallFont:
    case MiniFont:
        return m_smallFont;
    case FixedFont:
        return m_monospaceFont;
    case ToolButtonFont:
        return m_defaultFont;
    default:
        return m_defaultFont;
    }
}

//QIconEngine *MauiPlatformTheme::createIconEngine(const QString &iconName) const
//{
//    return new KIconEngine(iconName, KIconLoader::global());
//}

void MauiPlatformTheme::loadSettings()
{
    bool isDarkPreferred = m_theme->styleType() == 1;
    auto preferredScheme = m_theme->styleType() == 3 ? m_theme->customColorScheme() : (isDarkPreferred ? "NitruxDark" : "Nitrux");

    m_palette = new QPalette(loadColorScheme(preferredScheme, m_schemePath));

    m_style = "breeze";
    m_iconTheme = m_theme->iconTheme();
    m_iconSize = m_theme->iconSize();


    if(!m_defaultFont->fromString(m_theme->defaultFont()))
    {
        qWarning() << "Failed to set the default font" << m_theme->defaultFont();
    }

    if(! m_smallFont->fromString(m_theme->smallFont()))
    {
        qWarning() << "Failed to set the small font" << m_theme->defaultFont();
    }

    if(!m_monospaceFont->fromString(m_theme->monospacedFont()))
    {
        qWarning() << "Failed to set the monospaced font" << m_theme->defaultFont();
    }


    m_doubleClickInterval = QPlatformTheme::themeHint(QPlatformTheme::MouseDoubleClickInterval).toInt();
    //    m_doubleClickInterval = settings.value("double_click_interval", m_doubleClickInterval).toInt();
    m_cursorFlashTime = QPlatformTheme::themeHint(QPlatformTheme::CursorFlashTime).toInt();
    //    m_cursorFlashTime = settings.value("cursor_flash_time", m_cursorFlashTime).toInt();
    m_showShortcutsInContextMenus = false;
    m_buttonBoxLayout = QPlatformTheme::themeHint(QPlatformTheme::DialogButtonBoxLayout).toInt();
    //    m_buttonBoxLayout = settings.value("buttonbox_layout", m_buttonBoxLayout).toInt();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
    m_keyboardScheme = QPlatformTheme::themeHint(QPlatformTheme::KeyboardScheme).toInt();
    //    m_keyboardScheme = settings.value("keyboard_scheme", m_keyboardScheme).toInt();
#endif
    //    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, !settings.value("menus_have_icons", true).toBool());
    m_showIconsInMenus = true;
    m_toolButtonStyle = Qt::ToolButtonFollowStyle;
    m_wheelScrollLines = 3;

    m_singleClick = m_accessibility->singleClick();
    m_uiEffects = QPlatformTheme::themeHint(QPlatformTheme::UiEffects).toInt();
}

QList<QKeySequence> MauiPlatformTheme::keyBindings(QKeySequence::StandardKey key) const
{
    switch (key) {
    case QKeySequence::HelpContents:
        return KStandardShortcut::shortcut(KStandardShortcut::Help);
    case QKeySequence::WhatsThis:
        return KStandardShortcut::shortcut(KStandardShortcut::WhatsThis);
    case QKeySequence::Open:
        return KStandardShortcut::shortcut(KStandardShortcut::Open);
    case QKeySequence::Close:
        return KStandardShortcut::shortcut(KStandardShortcut::Close);
    case QKeySequence::Save:
        return KStandardShortcut::shortcut(KStandardShortcut::Save);
    case QKeySequence::New:
        return KStandardShortcut::shortcut(KStandardShortcut::New);
    case QKeySequence::Cut:
        return KStandardShortcut::shortcut(KStandardShortcut::Cut);
    case QKeySequence::Copy:
        return KStandardShortcut::shortcut(KStandardShortcut::Copy);
    case QKeySequence::Paste:
        return KStandardShortcut::shortcut(KStandardShortcut::Paste);
    case QKeySequence::Undo:
        return KStandardShortcut::shortcut(KStandardShortcut::Undo);
    case QKeySequence::Redo:
        return KStandardShortcut::shortcut(KStandardShortcut::Redo);
    case QKeySequence::Back:
        return KStandardShortcut::shortcut(KStandardShortcut::Back);
    case QKeySequence::Forward:
        return KStandardShortcut::shortcut(KStandardShortcut::Forward);
    case QKeySequence::Refresh:
        return KStandardShortcut::shortcut(KStandardShortcut::Reload);
    case QKeySequence::ZoomIn:
        return KStandardShortcut::shortcut(KStandardShortcut::ZoomIn);
    case QKeySequence::ZoomOut:
        return KStandardShortcut::shortcut(KStandardShortcut::ZoomOut);
    case QKeySequence::Print:
        return KStandardShortcut::shortcut(KStandardShortcut::Print);
    case QKeySequence::Find:
        return KStandardShortcut::shortcut(KStandardShortcut::Find);
    case QKeySequence::FindNext:
        return KStandardShortcut::shortcut(KStandardShortcut::FindNext);
    case QKeySequence::FindPrevious:
        return KStandardShortcut::shortcut(KStandardShortcut::FindPrev);
    case QKeySequence::Replace:
        return KStandardShortcut::shortcut(KStandardShortcut::Replace);
    case QKeySequence::SelectAll:
        return KStandardShortcut::shortcut(KStandardShortcut::SelectAll);
    case QKeySequence::MoveToNextWord:
        return KStandardShortcut::shortcut(KStandardShortcut::ForwardWord);
    case QKeySequence::MoveToPreviousWord:
        return KStandardShortcut::shortcut(KStandardShortcut::BackwardWord);
    case QKeySequence::MoveToNextPage:
        return KStandardShortcut::shortcut(KStandardShortcut::Next);
    case QKeySequence::MoveToPreviousPage:
        return KStandardShortcut::shortcut(KStandardShortcut::Prior);
    case QKeySequence::MoveToStartOfLine:
        return KStandardShortcut::shortcut(KStandardShortcut::BeginningOfLine);
    case QKeySequence::MoveToEndOfLine:
        return KStandardShortcut::shortcut(KStandardShortcut::EndOfLine);
    case QKeySequence::MoveToStartOfDocument:
        return KStandardShortcut::shortcut(KStandardShortcut::Begin);
    case QKeySequence::MoveToEndOfDocument:
        return KStandardShortcut::shortcut(KStandardShortcut::End);
    case QKeySequence::SaveAs:
        return KStandardShortcut::shortcut(KStandardShortcut::SaveAs);
    case QKeySequence::Preferences:
        return KStandardShortcut::shortcut(KStandardShortcut::Preferences);
    case QKeySequence::Quit:
        return KStandardShortcut::shortcut(KStandardShortcut::Quit);
    case QKeySequence::FullScreen:
        return KStandardShortcut::shortcut(KStandardShortcut::FullScreen);
    case QKeySequence::Deselect:
        return KStandardShortcut::shortcut(KStandardShortcut::Deselect);
    case QKeySequence::DeleteStartOfWord:
        return KStandardShortcut::shortcut(KStandardShortcut::DeleteWordBack);
    case QKeySequence::DeleteEndOfWord:
        return KStandardShortcut::shortcut(KStandardShortcut::DeleteWordForward);
    case QKeySequence::NextChild:
        return KStandardShortcut::shortcut(KStandardShortcut::TabNext);
    case QKeySequence::PreviousChild:
        return KStandardShortcut::shortcut(KStandardShortcut::TabPrev);
    case QKeySequence::Delete:
        return KStandardShortcut::shortcut(KStandardShortcut::MoveToTrash);
    default:
        return QPlatformTheme::keyBindings(key);
    }
}

bool MauiPlatformTheme::usePlatformNativeDialog(QPlatformTheme::DialogType type) const
{
    return type == QPlatformTheme::FileDialog && qobject_cast<QApplication *>(QCoreApplication::instance());
}

QString MauiPlatformTheme::standardButtonText(int button) const
{
    switch (static_cast<QPlatformDialogHelper::StandardButton>(button)) {
    case QPlatformDialogHelper::NoButton:
        qWarning() << Q_FUNC_INFO << "Unsupported standard button:" << button;
        return QString();
    case QPlatformDialogHelper::Ok:
        return KStandardGuiItem::ok().text();
    case QPlatformDialogHelper::Save:
        return KStandardGuiItem::save().text();
    case QPlatformDialogHelper::SaveAll:
        return i18nc("@action:button", "Save All");
    case QPlatformDialogHelper::Open:
        return KStandardGuiItem::open().text();
    case QPlatformDialogHelper::Yes:
        return i18nc("@action:button", "&Yes");
    case QPlatformDialogHelper::YesToAll:
        return i18nc("@action:button", "Yes to All");
    case QPlatformDialogHelper::No:
        return i18nc("@action:button", "&No");
    case QPlatformDialogHelper::NoToAll:
        return i18nc("@action:button", "No to All");
    case QPlatformDialogHelper::Abort:
        // FIXME KStandardGuiItem::stop() doesn't seem right here
        return i18nc("@action:button", "Abort");
    case QPlatformDialogHelper::Retry:
        return i18nc("@action:button", "Retry");
    case QPlatformDialogHelper::Ignore:
        return i18nc("@action:button", "Ignore");
    case QPlatformDialogHelper::Close:
        return KStandardGuiItem::close().text();
    case QPlatformDialogHelper::Cancel:
        return KStandardGuiItem::cancel().text();
    case QPlatformDialogHelper::Discard:
        return KStandardGuiItem::discard().text();
    case QPlatformDialogHelper::Help:
        return KStandardGuiItem::help().text();
    case QPlatformDialogHelper::Apply:
        return KStandardGuiItem::apply().text();
    case QPlatformDialogHelper::Reset:
        return KStandardGuiItem::reset().text();
    case QPlatformDialogHelper::RestoreDefaults:
        return KStandardGuiItem::defaults().text();
    default:
        return QPlatformTheme::defaultStandardButtonText(button);
    }
}

QPlatformDialogHelper *MauiPlatformTheme::createPlatformDialogHelper(QPlatformTheme::DialogType type) const
{
    return QPlatformTheme::createPlatformDialogHelper(type);

    //    switch (type) {
    //    case QPlatformTheme::FileDialog:
    //        if (useXdgDesktopPortal()) {
    //            return new QXdgDesktopPortalFileDialog;
    //        }
    //        return new KDEPlatformFileDialogHelper;
    //    case QPlatformTheme::FontDialog:
    //    case QPlatformTheme::ColorDialog:
    //    case QPlatformTheme::MessageDialog:
    //    default:
    //        return nullptr;
    //    }
}

//QPlatformSystemTrayIcon *MauiPlatformTheme::createPlatformSystemTrayIcon() const
//{
//    return new KDEPlatformSystemTrayIcon;
//}

QPlatformMenuBar *MauiPlatformTheme::createPlatformMenuBar() const
{
    //    if (isDBusGlobalMenuAvailable()) {
    //#ifndef KF6_TODO_DBUS_MENUBAR
    //        auto *menu = new QDBusMenuBar(const_cast<MauiPlatformTheme *>(this));

    //        QObject::connect(menu, &QDBusMenuBar::windowChanged, menu, [this, menu](QWindow *newWindow, QWindow *oldWindow) {
    //            const QString &serviceName = QDBusConnection::sessionBus().baseService();
    //            const QString &objectPath = menu->objectPath();

    //            setMenuBarForWindow(oldWindow, {}, {});
    //            setMenuBarForWindow(newWindow, serviceName, objectPath);
    //        });

    //        return menu;
    //#endif
    //    }

    return nullptr;
}

// force QtQuickControls2 to use the desktop theme as default
void MauiPlatformTheme::setQtQuickControlsTheme()
{
    // if the user is running only a QGuiApplication, explicitly unset the QQC1 desktop style and abort
    // as this style is all about QWidgets and we know setting this will make it crash
    if (!qobject_cast<QApplication *>(qApp)) {
        if (qgetenv("QT_QUICK_CONTROLS_1_STYLE").right(7) == "Desktop") {
            qunsetenv("QT_QUICK_CONTROLS_1_STYLE");
        }
        return;
    }
    // if the user has explicitly set something else, don't meddle
    // Also ignore the default Fusion style
    if (!QQuickStyle::name().isEmpty() && QQuickStyle::name() != QLatin1String("Fusion")) {
        return;
    }
    QQuickStyle::setStyle(QLatin1String("org.kde.desktop"));
}

QPalette MauiPlatformTheme::loadColorScheme(const QString &scheme, QString&path)
{
    path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes/") + scheme + QStringLiteral(".colors"));

    if (path.isEmpty())
    {
        qWarning() << "Could not find color scheme" << scheme << "falling back to BreezeLight";
        path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes/BreezeLight.colors"));
    }

    return QPalette(KColorScheme::createApplicationPalette(KSharedConfig::openConfig(path)));

}

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
Qt::ColorScheme MauiPlatformTheme::colorScheme() const
{
    return m_theme->styleType() == 1 ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light;
}
#elif QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
QPlatformTheme::Appearance MauiPlatformTheme::appearance() const
{
    return m_theme->styleType() == 1 ? Appearance::Dark : Appearance::Light;
}
#endif

#include "mauiplatformtheme.moc"
