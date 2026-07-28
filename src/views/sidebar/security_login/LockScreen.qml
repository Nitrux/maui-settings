// Copyright 2026 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.mauikit.filebrowsing as FB

Maui.ScrollColumn
{
    id: root

    readonly property var controller: (typeof desklockController !== "undefined"
                                       && desklockController) ? desklockController : null

    anchors.fill: parent
    spacing: Maui.Style.space.big

    function reloadSettings()
    {
        if (controller)
            controller.reload()
    }

    function saveSettings()
    {
        return true
    }

    function displayPath(path)
    {
        if (!path || !path.length)
            return i18n("No wallpaper selected")

        return path.replace(/^file:\/\//, "")
    }

    function pickWallpaper()
    {
        _wallpaperDialog.currentPath = controller && controller.wallpaperDirectory
            ? controller.wallpaperDirectory : FB.FM.homePath()
        _wallpaperDialog.browser.settings.viewType = FB.FMList.ICON_VIEW
        _wallpaperDialog.browser.settings.filterType = FB.FMList.IMAGE
        _wallpaperDialog.callback = (paths) =>
        {
            if (controller && paths && paths.length)
                controller.wallpaperPath = paths[0]
        }
        _wallpaperDialog.open()
    }

    FB.FileDialog
    {
        id: _wallpaperDialog
        singleSelection: true
        searchBar: true
        mode: FB.FileDialog.Modes.Open
        currentPath: controller && controller.wallpaperDirectory
            ? controller.wallpaperDirectory : FB.FM.homePath()
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Desklock Settings")
        text2: i18n("Configure the lock screen.")
        label2.wrapMode: Text.Wrap
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _appearanceLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _appearanceLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Appearance")
                text2: i18n("Choose the background and configure the date and time.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Wallpaper path")
                label1.elide: Text.ElideRight
                label2.text: controller ? root.displayPath(controller.wallpaperPath) : i18n("No wallpaper selected")
                label2.wrapMode: Text.Wrap

                template.content: Button
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    text: i18n("Choose")
                    onClicked: root.pickWallpaper()
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Time format")
                label1.elide: Text.ElideRight
                label2.text: i18n("Specify the display format for the time.")
                label2.wrapMode: Text.Wrap

                template.content: TextField
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }

                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 10
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    text: controller ? controller.timeFormat : "hh:mm"
                    placeholderText: "hh:mm"
                    onEditingFinished:
                    {
                        if (controller)
                            controller.timeFormat = text
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Date format")
                label1.elide: Text.ElideRight
                label2.text: i18n("Specify the display format for the date.")
                label2.wrapMode: Text.Wrap

                template.content: TextField
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }

                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 14
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                    text: controller ? controller.dateFormat : "dddd, dd MMMM yyyy"
                    placeholderText: "dddd, dd MMMM yyyy"
                    onEditingFinished:
                    {
                        if (controller)
                            controller.dateFormat = text
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Fade animations")
                label1.elide: Text.ElideRight
                label2.text: i18n("Animate the lock screen when it appears and unlocks.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }
                    checked: controller ? controller.fadeAnimationsEnabled : true
                    onToggled:
                    {
                        if (controller)
                            controller.fadeAnimationsEnabled = checked
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: controller ? controller.fadeAnimationsEnabled : true
                label1.text: i18n("Fade-in duration")
                label1.elide: Text.ElideRight
                label2.text: i18n("Animation duration in milliseconds.")
                label2.wrapMode: Text.Wrap

                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }

                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 8
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 6
                    from: 0
                    to: 5000
                    stepSize: 50
                    value: controller ? controller.fadeInDuration : 350
                    editable: true
                    onValueModified:
                    {
                        if (controller)
                            controller.fadeInDuration = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: controller ? controller.fadeAnimationsEnabled : true
                label1.text: i18n("Fade-out duration")
                label1.elide: Text.ElideRight
                label2.text: i18n("Unlock animation duration in milliseconds.")
                label2.wrapMode: Text.Wrap

                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }

                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 8
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 6
                    from: 0
                    to: 5000
                    stepSize: 50
                    value: controller ? controller.fadeOutDuration : 250
                    editable: true
                    onValueModified:
                    {
                        if (controller)
                            controller.fadeOutDuration = value
                    }
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _indicatorsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _indicatorsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("System Indicators")
                text2: i18n("Choose the status information shown while the session is locked.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("System resource monitors")
                label1.elide: Text.ElideRight
                label2.text: i18n("Show CPU, memory, and network activity.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }
                    checked: controller ? controller.showSystemMonitor : true
                    onToggled:
                    {
                        if (controller)
                            controller.showSystemMonitor = checked
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Battery indicator")
                label1.elide: Text.ElideRight
                label2.text: i18n("Show battery state and charge level when available.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }
                    checked: controller ? controller.showBattery : true
                    onToggled:
                    {
                        if (controller)
                            controller.showBattery = checked
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Media player")
                label1.elide: Text.ElideRight
                label2.text: i18n("Show the MPRIS media card and playback controls.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }
                    checked: controller ? controller.showMediaPlayer : true
                    onToggled:
                    {
                        if (controller)
                            controller.showMediaPlayer = checked
                    }
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _behaviorLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _behaviorLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Lock Behavior")
                text2: i18n("Configure timing values used by the session lock integration.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Idle lock timeout")
                label1.elide: Text.ElideRight
                label2.text: i18n("Seconds of inactivity before locking. Set to 0 to disable automatic locking.")
                label2.wrapMode: Text.Wrap

                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }

                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 8
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 6
                    from: 0
                    to: 86400
                    stepSize: 30
                    value: controller ? controller.idleLockTimeout : 300
                    editable: true
                    onValueModified:
                    {
                        if (controller)
                            controller.idleLockTimeout = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Password grace period")
                label1.elide: Text.ElideRight
                label2.text: i18n("Seconds after locking before a password is required. Set to 0 to require it immediately.")
                label2.wrapMode: Text.Wrap

                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }

                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 8
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 6
                    from: 0
                    to: 3600
                    stepSize: 5
                    value: controller ? controller.gracePeriod : 0
                    editable: true
                    onValueModified:
                    {
                        if (controller)
                            controller.gracePeriod = value
                    }
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        visible: controller && controller.lastError.length > 0
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.negativeBackgroundColor
        border.width: 1
        implicitHeight: _errorLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _errorLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Configuration Error")
                text2: controller ? controller.lastError : ""
                label2.wrapMode: Text.Wrap
            }
        }
    }
}
