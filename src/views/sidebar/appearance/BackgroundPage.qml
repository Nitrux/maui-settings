import QtQuick
import QtQuick.Effects
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.mauikit.filebrowsing as FB

Maui.ScrollColumn
{
    id: root
    readonly property var info: (typeof backgroundInfo !== "undefined" && backgroundInfo) ? backgroundInfo : null
    readonly property var displayController: (typeof displaysController !== "undefined" && displaysController) ? displaysController : null
    readonly property var previewDisplay: {
        const monitors = displayController ? displayController.monitors : []
        const monitorName = info && info.wallpaperMonitor ? info.wallpaperMonitor.trim() : ""
        if (monitorName.length)
            for (const monitor of monitors)
                if (String(monitor.name) === monitorName)
                    return monitor
        return monitors.length ? monitors[0] : null
    }
    readonly property real displayAspectRatio: {
        const display = previewDisplay
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
    property var monitorOptions: []

    property var fitModeLabels: [
        i18n("Cover"),
        i18n("Contain"),
        i18n("Tile"),
        i18n("Fill")
    ]
    property var fitModeValues: ["cover", "contain", "tile", "fit"]

    property var wallpaperOrderLabels: [
        i18n("Default"),
        i18n("Random"),
        i18n("Random shuffle")
    ]
    property var wallpaperOrderValues: ["default", "random", "random-shuffle"]

    function reloadSettings()
    {
        if (info)
            info.reload()
    }

    function refreshMonitorOptions()
    {
        const options = [{name: "", display: i18n("All outputs")}]
        const monitors = root.displayController ? root.displayController.monitors : []

        for (const monitor of monitors)
        {
            const name = String(monitor.name || "").trim()
            if (!name || options.some(option => option.name === name))
                continue

            options.push({name: name, display: String(monitor.display || name)})
        }

        const configured = root.info ? String(root.info.wallpaperMonitor || "").trim() : ""
        if (configured.length && !options.some(option => option.name === configured))
            options.push({name: configured, display: i18n("%1 (not detected)", configured)})

        root.monitorOptions = options
    }

    function saveSettings()
    {
        if (info)
            info.save()
    }

    function indexForValue(model, value)
    {
        for (let i = 0; i < model.length; ++i)
        {
            if (model[i] === value)
                return i
        }

        return 0
    }

    function monitorIndex(value)
    {
        const needle = String(value || "").trim()
        for (let i = 0; i < monitorOptions.length; ++i)
            if (monitorOptions[i].name === needle)
                return i

        return 0
    }

    function displayPath(path)
    {
        if (!path || !path.length)
            return i18n("No wallpaper selected")

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
        _fileDialog.currentPath = info && info.wallpaperDirectory ? info.wallpaperDirectory : FB.FM.homePath()
        _fileDialog.browser.settings.viewType = FB.FMList.ICON_VIEW
        _fileDialog.browser.settings.filterType = FB.FMList.IMAGE
        _fileDialog.callback = (paths) =>
        {
            if (!paths || !paths.length)
                return

            info.wallpaperPath = paths[0]
        }
        _fileDialog.open()
    }

    Component.onCompleted: root.refreshMonitorOptions()

    Connections
    {
        target: root.displayController
        function onMonitorsChanged() { root.refreshMonitorOptions() }
    }

    Connections
    {
        target: root.info
        function onSettingsChanged() { root.refreshMonitorOptions() }
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Background")
        text2: !root.info || !root.info.available
            ? i18n("hyprpaper is not available.")
            : i18n("Select a wallpaper, tune hyprpaper options, and save the generated config.")
    }

    FB.FileDialog
    {
        id: _fileDialog
        singleSelection: true
        searchBar: true
        mode: FB.FileDialog.Modes.Open
        currentPath: info && info.wallpaperDirectory ? info.wallpaperDirectory : FB.FM.homePath()
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
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
                    source: root.previewSource(root.info ? root.info.wallpaperPath : "")
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
                        : (root.info && root.info.wallpaperPath && root.info.wallpaperPath.length
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
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _wallpaperLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _wallpaperLayout
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
                text1: i18n("Wallpaper")
                text2: i18n("Choose an image or a folder for hyprpaper to use.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Wallpaper path")
                label1.elide: Text.ElideRight
                label2.text: info ? displayPath(info.wallpaperPath) : i18n("No wallpaper selected")
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
                    onClicked: pickWallpaper()
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Monitor")
                label1.elide: Text.ElideRight
                label2.text: i18n("Choose an output, or apply the wallpaper to all outputs.")
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
                    model: root.monitorOptions.map((option) => option.display)
                    currentIndex: root.monitorIndex(root.info ? root.info.wallpaperMonitor : "")
                    onActivated:
                    {
                        if (root.info && currentIndex >= 0 && currentIndex < root.monitorOptions.length)
                            root.info.wallpaperMonitor = root.monitorOptions[currentIndex].name
                    }
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _optionsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _optionsLayout
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
                text1: i18n("Wallpaper Options")
                text2: i18n("These map directly to hyprpaper's wallpaper block.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Fit mode")
                label1.elide: Text.ElideRight
                label2.text: i18n("Cover, contain, tile, or fill.")
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
                    model: fitModeLabels
                    currentIndex: info ? indexForValue(fitModeValues, info.wallpaperFitMode) : 0
                    onActivated:
                    {
                        if (info)
                            info.wallpaperFitMode = fitModeValues[currentIndex]
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Order")
                label1.elide: Text.ElideRight
                label2.text: i18n("Used when a folder contains multiple wallpapers.")
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
                    model: wallpaperOrderLabels
                    currentIndex: info ? indexForValue(wallpaperOrderValues, info.wallpaperOrder) : 0
                    onActivated:
                    {
                        if (info)
                            info.wallpaperOrder = wallpaperOrderValues[currentIndex]
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Timeout")
                label1.elide: Text.ElideRight
                label2.text: i18n("Seconds before hyprpaper changes to the next image.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 0
                    to: 9999
                    value: info ? info.wallpaperTimeout : 0
                    onValueModified:
                    {
                        if (info)
                            info.wallpaperTimeout = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Recursive")
                label1.elide: Text.ElideRight
                label2.text: i18n("Scan subfolders when the wallpaper path is a directory.")
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
                    checked: info ? info.wallpaperRecursive : false
                    onToggled:
                    {
                        if (info)
                            info.wallpaperRecursive = checked
                    }
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _runtimeLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _runtimeLayout
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
                text1: i18n("Runtime Options")
                text2: i18n("Hyprpaper global behavior toggles.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Splash")
                label1.elide: Text.ElideRight
                label2.text: i18n("Enable or disable the splash text overlay.")
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
                    checked: info ? info.splashEnabled : false
                    onToggled:
                    {
                        if (info)
                            info.splashEnabled = checked
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Splash offset")
                label1.elide: Text.ElideRight
                label2.text: i18n("Vertical offset in pixels for the splash text.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: -1000
                    enabled: info ? info.splashEnabled : false
                    to: 1000
                    value: info ? info.splashOffset : 20
                    onValueModified:
                    {
                        if (info)
                            info.splashOffset = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Splash opacity")
                label1.elide: Text.ElideRight
                label2.text: i18n("Splash text opacity as a percentage.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 0
                    to: 100
                    enabled: info ? info.splashEnabled : false
                    value: info ? Math.round(info.splashOpacity * 100) : 80
                    onValueModified:
                    {
                        if (info)
                            info.splashOpacity = value / 100.0
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("IPC")
                label1.elide: Text.ElideRight
                label2.text: i18n("Keep hyprpaper socket controls available.")
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
                    checked: info ? info.ipcEnabled : true
                    onToggled:
                    {
                        if (info)
                            info.ipcEnabled = checked
                    }
                }
            }
        }
    }

}
