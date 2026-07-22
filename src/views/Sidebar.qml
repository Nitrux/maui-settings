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
    required property string filterQuery
    property var groups: []

    signal sectionSelected(string section)

    function matchesFilter(text)
    {
        const query = filterQuery.trim().toLowerCase()
        if (!query.length)
            return true

        return String(text).toLowerCase().indexOf(query) >= 0
    }

    function visibleItemCount()
    {
        const query = filterQuery.trim().toLowerCase()
        if (!query.length)
        {
            let total = 0
            for (let i = 0; i < control.groups.length; ++i)
                total += control.groups[i].items.length
            return total
        }

        let count = 0
        for (let i = 0; i < control.groups.length; ++i)
        {
            const group = control.groups[i]
            if (matchesFilter(group.title))
            {
                count += group.items.length
                continue
            }

            for (let j = 0; j < group.items.length; ++j)
            {
                if (matchesFilter(group.items[j].label))
                    ++count
            }
        }
        return count
    }

    function groupVisible(group)
    {
        if (matchesFilter(group.title))
            return true

        for (let i = 0; i < group.items.length; ++i)
        {
            if (matchesFilter(group.items[i].label))
                return true
        }

        return false
    }

    Component
    {
        id: generalGroupComponent
        GeneralGroup {}
    }

    Component
    {
        id: appearanceGroupComponent
        AppearanceGroup {}
    }

    Component
    {
        id: desktopShellGroupComponent
        DesktopShellGroup {}
    }

    Component
    {
        id: applicationsGroupComponent
        ApplicationsGroup {}
    }

    Component
    {
        id: connectivityGroupComponent
        ConnectivityGroup {}
    }

    Component
    {
        id: hardwareSoundGroupComponent
        HardwareSoundGroup {}
    }

    Component
    {
        id: securityLoginGroupComponent
        SecurityLoginGroup {}
    }

    Component.onCompleted:
    {
        groups = [
            generalGroupComponent.createObject(control),
            appearanceGroupComponent.createObject(control),
            desktopShellGroupComponent.createObject(control),
            applicationsGroupComponent.createObject(control),
            connectivityGroupComponent.createObject(control),
            hardwareSoundGroupComponent.createObject(control),
            securityLoginGroupComponent.createObject(control)
        ]
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

            contentItem: ColumnLayout
            {
                anchors.fill: parent
                anchors.margins: Maui.Style.contentMargins
                spacing: Maui.Style.space.small

                Maui.SearchField
                {
                    Layout.fillWidth: true
                    placeholderText: i18n("Filter modules")
                    icon.source: "view-filter"
                    text: control.filterQuery
                    onTextEdited: control.filterQuery = text
                    onCleared: control.filterQuery = ""
                }

                ScrollView
                {
                    id: _scrollView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    focus: false
                    padding: 0
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOff

                    ColumnLayout
                    {
                        width: _scrollView.availableWidth
                        spacing: Maui.Style.space.medium

                        Repeater
                        {
                            model: control.groups

                            delegate: ColumnLayout
                            {
                                required property var modelData
                                Layout.fillWidth: true
                                spacing: Maui.Style.space.small
                                visible: control.groupVisible(modelData)

                                Maui.SectionHeader
                                {
                                    Layout.fillWidth: true
                                    text1: modelData.title
                                    label1.font.weight: Font.Bold
                                    label1.font.pixelSize: 14
                                    label2.visible: false
                                }

                                ColumnLayout
                                {
                                    Layout.fillWidth: true
                                    spacing: Maui.Style.space.small

                                    Repeater
                                    {
                                        model: modelData.items

                                        delegate: Maui.ListDelegate
                                        {
                                            required property var modelData
                                            readonly property bool shown: control.matchesFilter(modelData.label)
                                            Layout.fillWidth: true
                                            visible: shown
                                            enabled: shown
                                            iconSize: Maui.Style.iconSize
                                            iconVisible: true
                                            label: modelData.label
                                            iconName: modelData.icon
                                            isCurrentItem: modelData.section === control.currentSection

                                            onClicked:
                                            {
                                                control.sectionSelected(modelData.section)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
