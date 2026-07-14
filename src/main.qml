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

    property string currentSection: "general"
    property string searchQuery: ""

    function sectionTitle(section)
    {
        switch (section)
        {
        case "appearance":
            return i18n("Appearance")
        case "privacy":
            return i18n("Privacy")
        case "about":
            return i18n("About")
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
                onSectionSelected:
                {
                    root.currentSection = section
                    shell.sideBar.close()
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
