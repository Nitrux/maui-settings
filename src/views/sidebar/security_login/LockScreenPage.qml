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
    property var indicatorIconModeLabels: [i18n("System icons"), i18n("Nerd Font symbols")]
    property var indicatorIconModeValues: ["system", "nerd"]

    anchors.fill: parent
    spacing: Maui.Style.space.big

    function reloadSettings()
    {
        if (controller)
            controller.reload()
    }

    function saveSettings()
    {
        return controller ? controller.save() : false
    }

    function displayPath(path)
    {
        if (!path || !path.length)
            return i18n("No wallpaper selected")

        return path.replace(/^file:\/\//, "")
    }

    function indexForValue(model, value)
    {
        const needle = (value || "").trim()
        for (let i = 0; i < model.length; ++i)
            if (String(model[i]).trim() === needle)
                return i
        return -1
    }

    function displayAvatarPath(path)
    {
        const value = (path || "").trim()
        if (!value.length || value === "~/.face" || value === "~/.face.icon"
            || value === "%h/.face" || value === "%h/.face.icon")
            return i18n("Automatic")

        return root.displayPath(value)
    }

    function previewSource(path)
    {
        if (!path || !path.length)
            return ""

        return path.startsWith("file:") ? path : "file://" + path
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

    function pickAvatar()
    {
        _avatarDialog.currentPath = controller && controller.avatarDirectory
            ? controller.avatarDirectory : FB.FM.homePath()
        _avatarDialog.browser.settings.viewType = FB.FMList.ICON_VIEW
        _avatarDialog.browser.settings.filterType = FB.FMList.IMAGE
        _avatarDialog.callback = (paths) =>
        {
            if (controller && paths && paths.length)
                controller.avatarPath = paths[0]
        }
        _avatarDialog.open()
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

    FB.FileDialog
    {
        id: _avatarDialog
        singleSelection: true
        searchBar: true
        mode: FB.FileDialog.Modes.Open
        currentPath: controller && controller.avatarDirectory
            ? controller.avatarDirectory : FB.FM.homePath()
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Desklock Settings")
        text2: !root.controller || !root.controller.available
            ? i18n("desklock is not available.")
            : i18n("Configure the lock screen.")
        label2.wrapMode: Text.Wrap
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.controller ? root.controller.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        clip: true
        implicitHeight: _previewLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _previewLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Wallpaper Preview")
                text2: root.controller
                    ? root.displayPath(root.controller.wallpaperPath)
                    : i18n("No wallpaper selected")
                label2.wrapMode: Text.Wrap
            }

            Rectangle
            {
                Layout.fillWidth: true
                Layout.preferredHeight: Maui.Style.units.gridUnit * 12
                color: Maui.Theme.backgroundColor
                radius: Maui.Style.radiusV
                clip: true

                Image
                {
                    id: wallpaperPreviewImage
                    anchors.fill: parent
                    asynchronous: true
                    fillMode: Image.PreserveAspectCrop
                    source: root.previewSource(root.controller ? root.controller.wallpaperPath : "")
                }

                Label
                {
                    anchors.centerIn: parent
                    width: parent.width - Maui.Style.contentMargins * 2
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    visible: wallpaperPreviewImage.status !== Image.Ready
                    text: wallpaperPreviewImage.status === Image.Loading
                        ? i18n("Loading preview...")
                        : (root.controller && root.controller.wallpaperPath && root.controller.wallpaperPath.length
                            ? i18n("No wallpaper preview available.")
                            : i18n("No wallpaper selected."))
                    opacity: 0.7
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.controller ? root.controller.available : false
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
                label1.text: i18n("Indicator icon style")
                label1.elide: Text.ElideRight
                label2.text: i18n("Use system icon-theme artwork or Nerd Font symbols.")
                label2.wrapMode: Text.Wrap

                template.content: ComboBox
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: root.indicatorIconModeLabels
                    currentIndex: controller ? root.indexForValue(root.indicatorIconModeValues, controller.iconMode) : 0
                    onActivated: if (controller) controller.iconMode = root.indicatorIconModeValues[currentIndex]
                }
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
                label1.text: i18n("Lowercase date")
                label1.elide: Text.ElideRight
                label2.text: i18n("Display the formatted date using lowercase letters.")
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
                    checked: controller ? controller.lowercaseDate : false
                    onToggled: if (controller) controller.lowercaseDate = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Background blur")
                label1.elide: Text.ElideRight
                label2.text: i18n("Blur the selected wallpaper behind the lock screen.")
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
                    checked: controller ? controller.blurEnabled : true
                    onToggled: if (controller) controller.blurEnabled = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Background overlay")
                label1.elide: Text.ElideRight
                label2.text: i18n("Draw a theme-colored overlay above the wallpaper.")
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
                    checked: controller ? controller.overlayEnabled : true
                    onToggled: if (controller) controller.overlayEnabled = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: controller ? controller.overlayEnabled : true
                label1.text: i18n("Overlay opacity")
                label1.elide: Text.ElideRight
                label2.text: i18n("Background overlay opacity as a percentage.")
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
                    to: 100
                    value: controller ? Math.round(controller.overlayOpacity * 100) : 76
                    editable: true
                    onValueModified: if (controller) controller.overlayOpacity = value / 100.0
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.controller ? root.controller.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _sessionLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _sessionLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Session and User Options")
                text2: i18n("Configure the user identity shown on the lock screen.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Avatar image")
                label1.elide: Text.ElideRight
                label2.text: root.displayAvatarPath(
                    controller ? controller.avatarPath : "")
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
                    onClicked: root.pickAvatar()
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.controller ? root.controller.available : false
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

            // Maui.SectionItem
            // {
            //     Layout.fillWidth: true
            //     flat: true
            //     enabled: controller ? controller.showSystemMonitor : true
            //     label1.text: i18n("System monitor interval")
            //     label1.elide: Text.ElideRight
            //     label2.text: i18n("Polling interval in milliseconds.")
            //     label2.wrapMode: Text.Wrap

            //     template.content: SpinBox
            //     {
            //         property Item wideParent
            //         property Item responsiveSectionItem
            //         readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
            //         function updateResponsiveParent()
            //         {
            //             if (!wideParent || !responsiveSectionItem)
            //                 return
            //             parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
            //         }
            //         onResponsiveNarrowChanged: updateResponsiveParent()
            //         Component.onCompleted:
            //         {
            //             const originalParent = parent
            //             responsiveSectionItem = originalParent.parent.parent.parent
            //             wideParent = originalParent
            //             updateResponsiveParent()
            //         }
            //         Layout.fillWidth: responsiveNarrow
            //         Layout.minimumWidth: responsiveNarrow ? 0 : -1
            //         Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 8
            //         Layout.preferredWidth: Maui.Style.units.gridUnit * 6
            //         from: 1000
            //         to: 3600000
            //         stepSize: 1000
            //         value: controller ? controller.systemMonitorUpdateInterval : 3000
            //         editable: true
            //         onValueModified: if (controller) controller.systemMonitorUpdateInterval = value
            //     }
            // }

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

            // Maui.SectionItem
            // {
            //     Layout.fillWidth: true
            //     flat: true
            //     enabled: controller ? controller.showBattery : true
            //     label1.text: i18n("Battery check interval")
            //     label1.elide: Text.ElideRight
            //     label2.text: i18n("Polling interval in milliseconds.")
            //     label2.wrapMode: Text.Wrap

            //     template.content: SpinBox
            //     {
            //         property Item wideParent
            //         property Item responsiveSectionItem
            //         readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
            //         function updateResponsiveParent()
            //         {
            //             if (!wideParent || !responsiveSectionItem)
            //                 return
            //             parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
            //         }
            //         onResponsiveNarrowChanged: updateResponsiveParent()
            //         Component.onCompleted:
            //         {
            //             const originalParent = parent
            //             responsiveSectionItem = originalParent.parent.parent.parent
            //             wideParent = originalParent
            //             updateResponsiveParent()
            //         }
            //         Layout.fillWidth: responsiveNarrow
            //         Layout.minimumWidth: responsiveNarrow ? 0 : -1
            //         Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 8
            //         Layout.preferredWidth: Maui.Style.units.gridUnit * 6
            //         from: 1000
            //         to: 3600000
            //         stepSize: 1000
            //         value: controller ? controller.batteryUpdateInterval : 30000
            //         editable: true
            //         onValueModified: if (controller) controller.batteryUpdateInterval = value
            //     }
            // }

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
        enabled: root.controller ? root.controller.available : false
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
                label1.text: i18n("Hide cursor")
                label1.elide: Text.ElideRight
                label2.text: i18n("Hide the pointer while the lock screen is active.")
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
                    checked: controller ? controller.hideCursor : true
                    onToggled: if (controller) controller.hideCursor = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable idle lock")
                label1.elide: Text.ElideRight
                label2.text: i18n("Automatically lock the session after the configured period.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    checked: controller ? controller.idleLockEnabled : true
                    onToggled:
                    {
                        if (!controller)
                            return
                        if (checked && controller.idleLockTimeout === 0)
                            controller.idleLockTimeout = 350
                        controller.idleLockEnabled = checked
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: controller ? controller.idleLockEnabled : true
                label1.text: i18n("Dim screen timeout")
                label2.text: i18n("Seconds of inactivity before dimming the screen.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    from: 0
                    to: 86400
                    stepSize: 30
                    value: controller ? controller.dimTimeout : 300
                    editable: true
                    onValueModified: if (controller) controller.dimTimeout = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Idle lock timeout")
                enabled: controller ? controller.idleLockEnabled : true
                label1.elide: Text.ElideRight
                label2.text: i18n("Seconds of inactivity before locking.")
                label2.wrapMode: Text.Wrap

                template.content: SpinBox
                {
                    from: 0
                    to: 86400
                    stepSize: 30
                    value: controller ? controller.idleLockTimeout : 350
                    enabled: controller ? controller.idleLockEnabled : true
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
                enabled: controller ? controller.idleLockEnabled : true
                label1.text: i18n("Display off timeout")
                label2.text: i18n("Seconds of inactivity before turning off the display.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    from: 0
                    to: 86400
                    stepSize: 30
                    value: controller ? controller.dpmsTimeout : 500
                    editable: true
                    onValueModified: if (controller) controller.dpmsTimeout = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: controller ? controller.idleLockEnabled : true
                label1.text: i18n("Suspend timeout")
                label2.text: i18n("Seconds of inactivity before suspending the system.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    from: 0
                    to: 86400
                    stepSize: 30
                    value: controller ? controller.suspendTimeout : 600
                    editable: true
                    onValueModified: if (controller) controller.suspendTimeout = value
                }
            }
        }
    }
}
