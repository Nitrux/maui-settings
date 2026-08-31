import QtQuick
import QtQuick.Effects
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.mauikit.filebrowsing as FB

Maui.SettingsPage
{
    id: root

    readonly property var controller: (typeof qmlGreetController !== "undefined"
                                       && qmlGreetController) ? qmlGreetController : null
    readonly property var displayController: (typeof displaysController !== "undefined" && displaysController) ? displaysController : null
    readonly property var previewDisplay: {
        const monitors = displayController ? displayController.monitors : []
        return monitors.length ? monitors[0] : null
    }
    readonly property real displayAspectRatio: {
        const monitors = displayController ? displayController.monitors : []
        const display = monitors.length ? monitors[0] : null
        const width = display ? Number(display.width) : 0
        const height = display ? Number(display.height) : 0
        return width > 0 && height > 0 ? width / height : 16 / 9
    }
    readonly property real previewPadding: 18
    readonly property real previewSpacing: Maui.Style.space.small
    readonly property real layoutMinX: {
        let value = 0
        if (displayController)
            for (const monitor of displayController.monitors)
                value = Math.min(value, Number(monitor.x) || 0)
        return value
    }
    readonly property real layoutMinY: {
        let value = 0
        if (displayController)
            for (const monitor of displayController.monitors)
                value = Math.min(value, Number(monitor.y) || 0)
        return value
    }
    readonly property real layoutMaxX: {
        let value = 1
        if (displayController)
            for (const monitor of displayController.monitors)
                value = Math.max(value, (Number(monitor.x) || 0) + Math.max(1, Number(monitor.width) || 1))
        return value
    }
    readonly property real layoutMaxY: {
        let value = 1
        if (displayController)
            for (const monitor of displayController.monitors)
                value = Math.max(value, (Number(monitor.y) || 0) + Math.max(1, Number(monitor.height) || 1))
        return value
    }
    readonly property real layoutWidth: Math.max(1, layoutMaxX - layoutMinX)
    readonly property real layoutHeight: Math.max(1, layoutMaxY - layoutMinY)
    readonly property bool saveAvailable: controller ? controller.saveAvailable : false
    readonly property bool editable: saveAvailable && !controller.loading && !controller.saving
    property var indicatorIconModeLabels: [i18n("System icons"), i18n("Nerd Font symbols")]
    property var indicatorIconModeValues: ["system", "nerd"]

    anchors.fill: parent
    title: i18n("Greeter")
    spacing: Maui.Style.space.big

    function reloadSettings()
    {
        if (controller)
            controller.reload()
    }

    function saveSettings()
    {
        return saveAvailable ? controller.save() : false
    }

    function displayPath(path, emptyText)
    {
        if (!path || !path.length)
            return emptyText

        return path.replace(/^file:\/\//, "")
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
        text1: i18n("QMLGreet Settings")
        text2: !root.controller || !root.controller.available
            ? i18n("qmlgreet is not available.")
            : i18n("Configure the login screen.")
        label2.wrapMode: Text.Wrap
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.editable && root.controller ? root.controller.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        clip: true
        id: wallpaperPreviewCard
        implicitHeight: 230
        readonly property real previewContentTop: wallpaperPreviewHeader.y + wallpaperPreviewHeader.height + root.previewSpacing + Maui.Style.contentMargins
        readonly property real previewContentHeight: Math.max(1, height - previewContentTop - root.previewPadding)
        readonly property real previewScale: Math.min(
            (width - root.previewPadding * 2) / root.layoutWidth,
            previewContentHeight / root.layoutHeight)

        ColumnLayout
        {
            id: _previewLayout
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.topMargin: Maui.Style.contentMargins
            anchors.leftMargin: Maui.Style.contentMargins
            anchors.rightMargin: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                id: wallpaperPreviewHeader
                text1: i18n("Wallpaper Preview")
                text2: i18n("A preview of the selected wallpaper.")
                label2.wrapMode: Text.Wrap
            }

            Rectangle
            {
                Layout.alignment: Qt.AlignHCenter
                readonly property real displayWidth: root.previewDisplay && Number(root.previewDisplay.width) > 0
                    ? Number(root.previewDisplay.width) : root.displayAspectRatio
                readonly property real displayHeight: root.previewDisplay && Number(root.previewDisplay.height) > 0
                    ? Number(root.previewDisplay.height) : 1
                readonly property real previewWidth: Math.max(1, displayWidth * wallpaperPreviewCard.previewScale)
                readonly property real previewHeight: Math.max(1, displayHeight * wallpaperPreviewCard.previewScale)
                readonly property real previewSlack: Math.max(0, wallpaperPreviewCard.previewContentHeight - previewHeight)
                Layout.preferredWidth: previewWidth
                Layout.maximumWidth: parent.width
                Layout.preferredHeight: previewHeight
                Layout.topMargin: previewSlack / 2
                Layout.bottomMargin: root.previewPadding + previewSlack / 2
                color: Maui.Theme.backgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.textColor
                border.width: 1
                Image
                {
                    id: wallpaperPreviewImage
                    anchors.fill: parent
                    asynchronous: true
                    fillMode: Image.PreserveAspectCrop
                    layer.enabled: true
                    layer.effect: MultiEffect
                    {
                        maskEnabled: true
                        maskThresholdMin: 0.5
                        maskSpreadAtMin: 1.0
                        maskSpreadAtMax: 0.0
                        maskThresholdMax: 1.0
                        maskSource: ShaderEffectSource
                        {
                            sourceItem: Rectangle
                            {
                                width: wallpaperPreviewImage.width
                                height: wallpaperPreviewImage.height
                                radius: Maui.Style.radiusV
                                color: "white"
                            }
                        }
                    }
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
        enabled: root.editable && root.controller ? root.controller.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _appearanceLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _appearanceLayout
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.topMargin: Maui.Style.contentMargins
            anchors.leftMargin: Maui.Style.contentMargins
            anchors.rightMargin: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Appearance")
                text2: i18n("Choose the background, clock formats, theme, and visual effects.")
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
                    currentIndex: controller ? root.indicatorIconModeValues.indexOf(controller.iconMode) : 0
                    onActivated: if (controller) controller.iconMode = root.indicatorIconModeValues[currentIndex]
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Wallpaper path")
                label1.elide: Text.ElideRight
                label2.text: controller
                    ? root.displayPath(controller.wallpaperPath, i18n("No wallpaper selected"))
                    : i18n("No wallpaper selected")
                label2.wrapMode: Text.Wrap

                template.content: Button
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
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
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 10
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
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 14
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                    text: controller && controller.dateFormat !== "dddd, dd MMMM yyyy"
                        ? controller.dateFormat : ""
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
                label1.text: i18n("Background blur")
                label1.elide: Text.ElideRight
                label2.text: i18n("Blur the selected wallpaper behind the login controls.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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
                label2.text: i18n("Opacity of the background overlay as a percentage.")
                label2.wrapMode: Text.Wrap

                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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
                    Layout.minimumWidth: 0
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 8
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 6
                    from: 0
                    to: 100
                    editable: true
                    value: controller ? Math.round(controller.overlayOpacity * 100) : 76
                    onValueModified:
                    {
                        if (controller)
                            controller.overlayOpacity = value / 100.0
                    }
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.editable && root.controller ? root.controller.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _sessionLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _sessionLayout
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.topMargin: Maui.Style.contentMargins
            anchors.leftMargin: Maui.Style.contentMargins
            anchors.rightMargin: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Session and User Options")
                text2: i18n("Choose the initial Wayland session and configure user presentation.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Default Wayland session")
                label1.elide: Text.ElideRight
                label2.text: i18n("Select a discovered session.")
                label2.wrapMode: Text.Wrap

                template.content: ComboBox
                {
                    Maui.Theme.colorSet: Maui.Theme.Button

                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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
                    Layout.minimumWidth: 0
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 16
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 14
                    model: controller ? controller.availableSessions : []
                    currentIndex: controller ? model.indexOf(controller.defaultSession) : -1
                    onActivated:
                    {
                        if (controller)
                            controller.defaultSession = currentText
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Show user avatars")
                label1.elide: Text.ElideRight
                label2.text: i18n("Display user avatar images in the account list.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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

                    checked: controller ? controller.showAvatars : true
                    onToggled: if (controller) controller.showAvatars = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: controller ? controller.showAvatars : true
                label1.text: i18n("Avatar image")
                label1.elide: Text.ElideRight
                label2.text: controller && controller.avatarPath.length
                    ? root.displayPath(controller.avatarPath, i18n("Automatic"))
                    : i18n("Automatic")
                label2.wrapMode: Text.Wrap

                template.content: Button
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    text: i18n("Choose")
                    onClicked: root.pickAvatar()
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Remember last logged-in user")
                label1.elide: Text.ElideRight
                label2.text: i18n("Preselect the user who most recently signed in.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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

                    checked: controller ? controller.rememberLastUser : true
                    onToggled: if (controller) controller.rememberLastUser = checked
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.editable && root.controller ? root.controller.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _indicatorsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _indicatorsLayout
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.topMargin: Maui.Style.contentMargins
            anchors.leftMargin: Maui.Style.contentMargins
            anchors.rightMargin: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("System Indicators")
                text2: i18n("Control the information shown alongside the login controls.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Battery indicator")
                label1.elide: Text.ElideRight
                label2.text: i18n("Show battery charge and charging status when a battery is available.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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
                    onToggled: if (controller) controller.showBattery = checked
                }
            }
        }
    }

}
