import QtQuick
import QtCore
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

import "views"

Maui.ApplicationWindow
{
    id: root
    title: root.sectionTitle(root.currentSection)
    color: "transparent"
    background: null

    property string currentSection: "general-about"
    property string filterQuery: ""

    function currentSettingsPage()
    {
        switch (root.currentSection)
        {
        case "appearance-background":
            return _backgroundPageLoader.item
        case "appearance-theme":
            return _themePageLoader.item
        case "desktop-valenz":
            return _valenzPageLoader.item
        case "desktop-nudge-osd":
            return _nudgeOsdPageLoader.item
        case "desktop-marina":
            return _marinaPageLoader.item
        default:
            return null
        }
    }

    function reloadCurrentSettings()
    {
        const settingsPage = currentSettingsPage()
        if (settingsPage && typeof settingsPage.reloadSettings === "function")
            settingsPage.reloadSettings()
    }

    function saveCurrentSettings()
    {
        const settingsPage = currentSettingsPage()
        if (settingsPage && typeof settingsPage.saveSettings === "function")
        {
            const saved = settingsPage.saveSettings()

            if (!saved)
                return

            if (root.currentSection === "desktop-marina")
                root.notify("view-refresh", i18n("Marina"), i18n("Restarting the desktop dock..."), [], false)
        }
    }

    function sectionTitle(section)
    {
        switch (section)
        {
        case "general-about":
            return i18n("About")
        case "appearance-background":
            return i18n("Background")
        case "appearance-theme":
            return i18n("Theme")
        case "desktop-valenz":
            return i18n("Valenz Settings")
        case "desktop-nudge-osd":
            return i18n("NudgeOSD Settings")
        case "desktop-marina":
            return i18n("Marina Settings")
        case "applications-defaults":
            return i18n("Defaults")
        case "applications-cache":
            return i18n("Cache")
        case "connectivity-bluetooth":
            return i18n("Bluetooth")
        case "connectivity-network":
            return i18n("Network")
        case "hardware-sound-audio":
            return i18n("Audio")
        case "hardware-sound-input":
            return i18n("Input Devices")
        case "hardware-sound-form-factor":
            return i18n("Form Factor")
        case "hardware-sound-accessibility":
            return i18n("Accessibility")
        case "security-login-greeter":
            return i18n("Greeter")
        case "security-login-lock-screen":
            return i18n("Lock Screen")
        default:
            return i18n("General")
        }
    }

    Maui.WindowBlur
    {
        view: root
        geometry: Qt.rect(0, 0, root.width, root.height)
        windowRadius: Maui.Style.radiusV
        enabled: true
    }

    Rectangle
    {
        anchors.fill: parent
        color: Maui.Theme.backgroundColor
        opacity: 0.76
        radius: Maui.Style.radiusV
    }

    Maui.SideBarView
    {
        id: shell
        anchors.fill: parent
        background: null

        Maui.Theme.colorSet: Maui.Theme.Window

        sideBar.preferredWidth: Maui.Style.units.gridUnit * 12
        sideBar.minimumWidth: Maui.Style.units.gridUnit * 12
        sideBar.autoHide: true
        sideBar.autoShow: true
        sideBar.floats: sideBar.collapsed

        sideBarContent: Item
        {
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            anchors.rightMargin: 0

            Sidebar
            {
                anchors.fill: parent
                anchors.rightMargin: 0
                currentSection: root.currentSection
                filterQuery: root.filterQuery
                onSectionSelected: (section) =>
                {
                    root.currentSection = section
                }
            }
        }

        Maui.PageLayout
        {
            id: page
            anchors.fill: parent
            clip: true
            background: null

            split: false
            splitIn: ToolBar.Header
            altHeader: Maui.Handy.isMobile
            Maui.Controls.showCSD: true

            headBar.visible: true
            headBar.forceCenterMiddleContent: false
            headerMargins: Maui.Handy.isMobile ? 0 : Maui.Style.contentMargins
            footerMargins: headerMargins

            Maui.Theme.colorSet: Maui.Theme.View

            headBar.leftContent: [
                Loader
                {
                    id: _sidebarToggleLoader
                    asynchronous: true
                    active: !!shell.sideBar
                    visible: active

                    sourceComponent: ToolButton
                    {
                        icon.name: shell.sideBar.visible ? "sidebar-collapse" : "sidebar-expand"
                        display: ToolButton.IconOnly
                        checked: shell.sideBar.visible
                        ToolTip.delay: 1000
                        ToolTip.timeout: 5000
                        ToolTip.visible: hovered
                        ToolTip.text: i18n("Toggle sidebar")
                        onClicked:
                        {
                            const nextVisible = !shell.sideBar.visible
                            if (nextVisible)
                                shell.sideBar.open()
                            else
                                shell.sideBar.close()
                        }
                    }
                },

                ToolSeparator
                {
                    visible: _sidebarToggleLoader.active
                    topPadding: 10
                    bottomPadding: 10
                },

                Loader
                {
                    id: _settingsActionsLoader
                    asynchronous: true
                    active: root.currentSection === "appearance-background" || root.currentSection === "appearance-theme" || root.currentSection === "desktop-valenz" || root.currentSection === "desktop-nudge-osd" || root.currentSection === "desktop-marina"
                    visible: active

                    sourceComponent: RowLayout
                    {
                        spacing: Maui.Style.space.small

                        ToolButton
                        {
                            icon.name: "view-refresh"
                            display: ToolButton.IconOnly
                            ToolTip.delay: 1000
                            ToolTip.timeout: 5000
                            ToolTip.visible: hovered
                            ToolTip.text: i18n("Reload settings")
                            onClicked: root.reloadCurrentSettings()
                        }

                        ToolButton
                        {
                            icon.name: "document-save"
                            display: ToolButton.IconOnly
                            ToolTip.delay: 1000
                            ToolTip.timeout: 5000
                            ToolTip.visible: hovered
                            ToolTip.text: i18n("Save settings")
                            onClicked: root.saveCurrentSettings()
                        }
                    }
                }
            ]

            headBar.middleContent: Item { implicitWidth: 0; implicitHeight: 0 }

            headBar.rightContent: [

                ToolSeparator
                {
                    topPadding: 10
                    bottomPadding: 10
                },

                Loader
                {
                    id: _mainMenuLoader
                    asynchronous: true
                    active: true
                    visible: true

                    sourceComponent: Maui.ToolButtonMenu
                    {
                        icon.name: "overflow-menu"

                        MenuItem
                        {
                            text: i18n("About")
                            icon.name: "documentinfo"
                            onTriggered: Maui.App.aboutDialog()
                        }
                    }
                }
            ]

            Item
            {
                anchors.fill: parent
                Item
                {
                    id: _contentArea
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom

                    Loader
                    {
                        anchors.fill: parent
                        active: root.currentSection === "general-about"
                        visible: active
                        source: active ? "views/sidebar/general/AboutPage.qml" : ""
                    }

                    Loader
                    {
                        id: _backgroundPageLoader
                        anchors.fill: parent
                        active: root.currentSection === "appearance-background"
                        visible: active
                        source: active ? "views/sidebar/appearance/BackgroundPage.qml" : ""
                    }

                    Loader
                    {
                        id: _themePageLoader
                        anchors.fill: parent
                        active: root.currentSection === "appearance-theme"
                        visible: active
                        source: active ? "views/sidebar/appearance/ThemePage.qml" : ""
                    }

                    Loader
                    {
                        id: _valenzPageLoader
                        anchors.fill: parent
                        active: root.currentSection === "desktop-valenz"
                        visible: active
                        source: active ? "views/sidebar/desktop_shell/ValenzPage.qml" : ""
                    }

                    Loader
                    {
                        id: _nudgeOsdPageLoader
                        anchors.fill: parent
                        active: root.currentSection === "desktop-nudge-osd"
                        visible: active
                        source: active ? "views/sidebar/desktop_shell/NudgeOsdPage.qml" : ""
                    }

                    Loader
                    {
                        id: _marinaPageLoader
                        anchors.fill: parent
                        active: root.currentSection === "desktop-marina"
                        visible: active
                        source: active ? "views/sidebar/desktop_shell/MarinaPage.qml" : ""
                    }

                    Maui.Holder
                    {
                        anchors.centerIn: parent
                        width: Math.min(parent.width - Maui.Style.contentMargins * 2, 520)
                        visible: root.currentSection !== "general-about" && root.currentSection !== "appearance-background" && root.currentSection !== "appearance-theme" && root.currentSection !== "desktop-valenz" && root.currentSection !== "desktop-nudge-osd" && root.currentSection !== "desktop-marina"
                        emoji: "documentinfo"
                        title: root.sectionTitle(root.currentSection)
                        body: i18n("This settings section is not implemented yet.")
                    }
                }
            }
        }
    }
}
