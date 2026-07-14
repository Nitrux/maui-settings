import QtQuick
import QtCore
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

import "views"

Maui.ApplicationWindow
{
    id: root
    title: i18n("Maui Settings")
    color: "transparent"
    background: null

    property string currentSection: "general-about"
    property string searchQuery: ""

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
        case "desktop-shell-desktop":
            return i18n("Desktop")
        case "desktop-shell-panel":
            return i18n("Panel")
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
                searchQuery: root.searchQuery
                groups: [
                    {
                        title: "General",
                        items: [
                            { label: "About", section: "general-about", icon: "documentinfo" }
                        ]
                    },
                    {
                        title: "Appearance",
                        items: [
                            { label: "Background", section: "appearance-background", icon: "preferences-desktop-wallpaper" },
                            { label: "Theme", section: "appearance-theme", icon: "preferences-desktop-theme" }
                        ]
                    },
                    {
                        title: "Desktop & Shell",
                        items: [
                            { label: "Desktop", section: "desktop-shell-desktop", icon: "computer" },
                            { label: "Panel", section: "desktop-shell-panel", icon: "view-media-sidebar" }
                        ]
                    },
                    {
                        title: "Applications",
                        items: [
                            { label: "Defaults", section: "applications-defaults", icon: "document-open" },
                            { label: "Cache", section: "applications-cache", icon: "edit-clear-history" }
                        ]
                    },
                    {
                        title: "Connectivity",
                        items: [
                            { label: "Bluetooth", section: "connectivity-bluetooth", icon: "bluetooth" },
                            { label: "Network", section: "connectivity-network", icon: "network-wireless" }
                        ]
                    },
                    {
                        title: "Hardware & Sound",
                        items: [
                            { label: "Audio", section: "hardware-sound-audio", icon: "audio-headphones" },
                            { label: "Input Devices", section: "hardware-sound-input", icon: "input-keyboard" },
                            { label: "Form Factor", section: "hardware-sound-form-factor", icon: "computer-laptop" },
                            { label: "Accessibility", section: "hardware-sound-accessibility", icon: "preferences-desktop-accessibility" }
                        ]
                    },
                    {
                        title: "Security & Login",
                        items: [
                            { label: "Greeter", section: "security-login-greeter", icon: "system-users" },
                            { label: "Lock Screen", section: "security-login-lock-screen", icon: "system-lock-screen" }
                        ]
                    }
                ]
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

            split: width < 800
            splitIn: ToolBar.Footer
            altHeader: Maui.Handy.isMobile
            Maui.Controls.showCSD: true

            headBar.visible: true
            headBar.forceCenterMiddleContent: true
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
                }
            ]

            headBar.middleContent: Loader
            {
                id: _searchFieldLoader
                asynchronous: true
                Layout.fillWidth: true
                Layout.minimumWidth: 100
                Layout.maximumWidth: 500
                Layout.alignment: Qt.AlignCenter

                sourceComponent: Maui.SearchField
                {
                    placeholderText: i18n("Search settings")
                    onTextEdited: root.searchQuery = text
                }
            }

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

        }
    }
}
