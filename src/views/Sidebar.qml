import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Loader
{
    id: control
    asynchronous: true
    active: (control.enabled && control.visible) || item
    Keys.enabled: false
    focus: false

    required property string currentSection
    required property string searchQuery

    signal sectionSelected(string section)

    function matchesFilter(label)
    {
        const query = searchQuery.trim().toLowerCase()
        if (!query.length)
            return true

        return String(label).toLowerCase().indexOf(query) >= 0
    }

    OpacityAnimator on opacity
    {
        from: 0
        to: 1
        duration: Maui.Style.units.longDuration
        running: control.status === Loader.Ready
    }

    sourceComponent: Item
    {
        anchors.fill: parent

        Pane
        {
            id: _sideBarPane
            anchors.fill: parent
            padding: 0
            focus: false
            clip: true
            Maui.Theme.colorSet: Maui.Theme.Window
            Maui.Theme.inherit: false

            background: Rectangle
            {
                color: Maui.Theme.alternateBackgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.backgroundColor
                border.width: 1
            }

            contentItem: Item
            {
                anchors.fill: parent
                anchors.margins: Maui.Style.contentMargins

                ColumnLayout
                {
                    anchors.fill: parent
                    spacing: Maui.Style.space.small

                    Maui.SectionHeader
                    {
                        Layout.fillWidth: true
                        text1: i18n("Settings")
                        label1.font.weight: Font.Bold
                        label1.font.pixelSize: 14
                        label2.visible: false
                    }

                    Maui.ListBrowser
                    {
                        id: _listBrowser
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        padding: 0
                        focus: false
                        clip: true
                        Keys.enabled: false
                        verticalScrollBarPolicy: ScrollBar.AlwaysOff

                        model: sectionsModel

                        holder.visible: count === 0
                        holder.title: i18n("Settings")
                        holder.body: i18n("Your sections will appear here")

                        delegate: Maui.ListDelegate
                        {
                            readonly property bool shown: control.matchesFilter(model.label)
                            width: _listBrowser.availableWidth
                            height: shown ? implicitHeight : 0
                            visible: shown
                            enabled: shown
                            iconSize: Maui.Style.iconSize
                            iconVisible: true
                            label: model.label
                            iconName: model.icon
                            isCurrentItem: model.section === control.currentSection

                            onClicked:
                            {
                                control.sectionSelected(model.section)
                            }
                        }
                    }
                }
            }

            ListModel
            {
                id: sectionsModel

                ListElement { label: "General"; section: "general"; icon: "settings-configure" }
                ListElement { label: "Appearance"; section: "appearance"; icon: "preferences-desktop-theme" }
                ListElement { label: "Privacy"; section: "privacy"; icon: "security-high" }
                ListElement { label: "About"; section: "about"; icon: "documentinfo" }
            }
        }
    }
}
