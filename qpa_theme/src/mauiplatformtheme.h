/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Kevin Ottens <ervin+bluesystems@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <qpa/qplatformtheme.h>
#include <qpa/qplatformdialoghelper.h>
#include <QHash>
#include <QKeySequence>

class QFont;
class QIconEngine;
class QWindow;

namespace MauiMan
{
class ThemeManager;
class AccessibilityManager;
}

class MauiPlatformTheme : public QObject, public QPlatformTheme
{
    Q_OBJECT
public:
    MauiPlatformTheme(QObject * parent = nullptr);
    ~MauiPlatformTheme() override;

    QVariant themeHint(ThemeHint hint) const override;
    QIcon fileIcon(const QFileInfo &fileInfo, QPlatformTheme::IconOptions iconOptions) const override;

    const QPalette *palette(Palette type = SystemPalette) const override;
    const QFont *font(Font type) const override;
//    QIconEngine *createIconEngine(const QString &iconName) const override;
    QList<QKeySequence> keyBindings(QKeySequence::StandardKey key) const override;

    QPlatformDialogHelper *createPlatformDialogHelper(DialogType type) const override;
    bool usePlatformNativeDialog(DialogType type) const override;

    QString standardButtonText(int button) const override;

    //    QPlatformSystemTrayIcon *createPlatformSystemTrayIcon() const override;

    QPlatformMenuBar *createPlatformMenuBar() const override;


    void windowCreated(QWindow *window);
private Q_SLOTS:
    void applySettings();
private:
    MauiMan::ThemeManager *m_theme;
      MauiMan::AccessibilityManager *m_accessibility;

    void loadSettings();
    void setQtQuickControlsTheme();

    //props
    QPalette *m_palette;
    QString m_style;
    QString m_iconTheme;

    int m_doubleClickInterval;
    int m_cursorFlashTime;
    int m_iconSize;

    bool m_showShortcutsInContextMenus;
    bool m_showIconsInMenus;
    bool m_usePalette = true;
    bool m_singleClick = false;
    int m_buttonBoxLayout;
    int m_keyboardScheme;
    int m_toolButtonStyle;
    int m_wheelScrollLines;

    int m_uiEffects;

    QFont *m_defaultFont;
    QFont *m_smallFont;
    QFont *m_monospaceFont;

    //helpers
    QPalette loadColorScheme(const QString &scheme);
    bool hasWidgets();
    void setupIconLoader();
    void iconChanged();

};

