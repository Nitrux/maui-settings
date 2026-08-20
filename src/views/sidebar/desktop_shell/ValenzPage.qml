import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.mauikit.filebrowsing as FB

Maui.ScrollColumn
{
    id: root
    readonly property var info: (typeof valenzInfo !== "undefined" && valenzInfo) ? valenzInfo : null
    readonly property var toma: (typeof tomaInfo !== "undefined" && tomaInfo) ? tomaInfo : null

    property var audioDeviceModel: []

    property var screenPlacementLabels: [i18n("Active screen"), i18n("All screens")]
    property var screenPlacementValues: ["active", "all"]
    property var iconModeLabels: [i18n("System icons"), i18n("Nerd Font symbols")]
    property var iconModeValues: ["system16", "nerd"]
    property var temperatureUnitLabels: [i18n("Celsius"), i18n("Fahrenheit")]
    property var temperatureUnitValues: ["celsius", "fahrenheit"]

    function refreshAudioDevices()
    {
        const devices = [{ name: "default", description: i18n("Default audio source") }]
        if (typeof audioController !== "undefined" && audioController && audioController.sources)
        {
            for (let i = 0; i < audioController.sources.length; ++i)
            {
                const source = audioController.sources[i]
                const name = source.name || ""
                if (!name.length || devices.some((device) => device.name === name))
                    continue

                devices.push({ name: name, description: source.description || name })
            }
        }

        if (root.toma && root.toma.audioDevice && !devices.some((device) => device.name === root.toma.audioDevice))
            devices.push({ name: root.toma.audioDevice, description: root.toma.audioDevice })
        root.audioDeviceModel = devices
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

    function reloadSettings()
    {
        if (info)
            info.reload()
        if (toma)
            toma.reload()
        root.refreshAudioDevices()
    }

    function saveSettings()
    {
        const valenzSaved = info ? info.save() : true
        const tomaSaved = toma ? toma.save() : true
        return valenzSaved && tomaSaved
    }

    Component.onCompleted: root.refreshAudioDevices()

    Connections
    {
        target: (typeof audioController !== "undefined") ? audioController : null
        function onSourcesChanged() { root.refreshAudioDevices() }
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    FB.FileDialog
    {
        id: _tomaFolderDialog
        singleSelection: true
        searchBar: true
        mode: FB.FileDialog.Modes.Open
        currentPath: FB.FM.homePath()
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Valenz Settings")
        text2: !root.info || !root.info.available
            ? i18n("valenz is not available.")
            : i18n("Configure the desktop bar.")
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _barLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _barLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Desktop Bar")
                text2: i18n("Control the bar size, screen placement, and layer-shell margins.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Screen placement")
                label1.elide: Text.ElideRight
                label2.text: i18n("Show Valenz on the active screen or on every screen.")
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
                    model: root.screenPlacementLabels
                    currentIndex: info ? root.indexForValue(root.screenPlacementValues, info.screenPlacement) : 0
                    onActivated: if (info) info.screenPlacement = root.screenPlacementValues[currentIndex]
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Bar height")
                label1.elide: Text.ElideRight
                label2.text: i18n("Total bar height in pixels.")
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
                    from: 1
                    to: 100
                    value: info ? info.barHeight : 56
                    onValueModified: if (info) info.barHeight = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Top spacing")
                label1.elide: Text.ElideRight
                label2.text: i18n("Top layer-shell margin in pixels.")
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
                    from: 0
                    to: 64
                    value: info ? info.barLayerSpacingTop : 0
                    onValueModified: if (info) info.barLayerSpacingTop = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Bottom spacing")
                label1.elide: Text.ElideRight
                label2.text: i18n("Bottom layer-shell margin in pixels.")
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
                    from: 0
                    to: 64
                    value: info ? info.barLayerSpacingBottom : 0
                    onValueModified: if (info) info.barLayerSpacingBottom = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Left spacing")
                label1.elide: Text.ElideRight
                label2.text: i18n("Left layer-shell margin in pixels.")
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
                    from: 0
                    to: 64
                    value: info ? info.barLayerSpacingLeft : 0
                    onValueModified: if (info) info.barLayerSpacingLeft = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Right spacing")
                label1.elide: Text.ElideRight
                label2.text: i18n("Right layer-shell margin in pixels.")
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
                    from: 0
                    to: 64
                    value: info ? info.barLayerSpacingRight : 0
                    onValueModified: if (info) info.barLayerSpacingRight = value
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
        implicitHeight: _controlCenterLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _controlCenterLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Control Center")
                text2: i18n("Configure the Control Center presentation and actions.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Icon style")
                label1.elide: Text.ElideRight
                label2.text: i18n("Use icon-theme artwork or Nerd Font symbols.")
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
                    model: root.iconModeLabels
                    currentIndex: info ? root.indexForValue(root.iconModeValues, info.controlCenterIconMode) : 0
                    onActivated: if (info) info.controlCenterIconMode = root.iconModeValues[currentIndex]
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Light color scheme")
                label1.elide: Text.ElideRight
                label2.text: i18n("Color scheme used when Dark mode is disabled in Valenz.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    text: info ? info.lightColorScheme : "CatppuccinLatteNitrux"
                    onEditingFinished: if (info) info.lightColorScheme = text
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Dark color scheme")
                label1.elide: Text.ElideRight
                label2.text: i18n("Color scheme used when Dark mode is enabled in Valenz.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    text: info ? info.darkColorScheme : "CatppuccinMochaNitrux"
                    onEditingFinished: if (info) info.darkColorScheme = text
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Disk usage path")
                label1.elide: Text.ElideRight
                label2.text: i18n("Filesystem path represented by the storage indicator.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    text: info ? info.controlCenterDiskUsagePath : "/"
                    onEditingFinished: if (info) info.controlCenterDiskUsagePath = text
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Power menu command")
                label1.elide: Text.ElideRight
                label2.text: i18n("Command launched by the power action.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    text: info ? info.controlCenterPowerCommand : "wlogout"
                    onEditingFinished: if (info) info.controlCenterPowerCommand = text
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Settings command")
                label1.elide: Text.ElideRight
                label2.text: i18n("Command launched by the settings action.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    text: info ? info.controlCenterSettingsCommand : "systemsettings"
                    onEditingFinished: if (info) info.controlCenterSettingsCommand = text
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
        implicitHeight: _actionsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _actionsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Actions")
                text2: i18n("Configure the commands launched by the bar action buttons.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Launcher command")
                label1.elide: Text.ElideRight
                label2.text: i18n("Command launched by the launcher action.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    text: info ? info.launcherCommand : "vicinae toggle"
                    onEditingFinished: if (info) info.launcherCommand = text
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Clipboard command")
                label1.elide: Text.ElideRight
                label2.text: i18n("Command launched by the clipboard action.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    text: info ? info.clipboardCommand : "vicinae vicinae://launch/clipboard/history"
                    onEditingFinished: if (info) info.clipboardCommand = text
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
        implicitHeight: _mediaLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _mediaLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Media")
                text2: i18n("Configure the media controls shown in the bar.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Always show media controls")
                label1.elide: Text.ElideRight
                label2.text: i18n("Keep the media section visible when nothing is playing.")
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
                    checked: info ? info.mprisAlwaysVisible : false
                    onToggled: if (info) info.mprisAlwaysVisible = checked
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
        implicitHeight: _weatherLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _weatherLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Weather")
                text2: i18n("Set the location and update behavior for weather information.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Latitude")
                label1.elide: Text.ElideRight
                label2.text: i18n("Location latitude from -90 to 90.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 11
                    text: info ? String(info.weatherLatitude) : ""
                    validator: DoubleValidator { bottom: -90; top: 90; notation: DoubleValidator.StandardNotation }
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    onEditingFinished: if (info && acceptableInput) info.weatherLatitude = Number(text)
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Longitude")
                label1.elide: Text.ElideRight
                label2.text: i18n("Location longitude from -180 to 180.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 11
                    text: info ? String(info.weatherLongitude) : ""
                    validator: DoubleValidator { bottom: -180; top: 180; notation: DoubleValidator.StandardNotation }
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    onEditingFinished: if (info && acceptableInput) info.weatherLongitude = Number(text)
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Temperature unit")
                label1.elide: Text.ElideRight
                label2.text: i18n("Unit used for the current temperature.")
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 11
                    model: root.temperatureUnitLabels
                    currentIndex: info ? root.indexForValue(root.temperatureUnitValues, info.weatherTemperatureUnit) : 0
                    onActivated: if (info) info.weatherTemperatureUnit = root.temperatureUnitValues[currentIndex]
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Refresh interval")
                label1.elide: Text.ElideRight
                label2.text: i18n("Minutes between weather updates.")
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
                    from: 5
                    to: 180
                    value: info ? info.weatherRefreshMinutes : 20
                    onValueModified: if (info) info.weatherRefreshMinutes = value
                }
            }
        }
    }
    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.toma ? root.toma.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _tomaLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _tomaLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Screenshots and Screen Recording")
                text2: i18n("Choose capture locations and recording options for toma.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Screenshot location")
                label1.elide: Text.ElideRight
                label2.text: root.toma ? root.toma.screenshotsPath : i18n("No screenshot location selected.")
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
                    onClicked:
                    {
                        _tomaFolderDialog.currentPath = root.toma ? root.toma.screenshotsPath : FB.FM.homePath()
                        _tomaFolderDialog.callback = (paths) =>
                        {
                            if (root.toma && paths && paths.length)
                                root.toma.screenshotsPath = paths[0]
                        }
                        _tomaFolderDialog.open()
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Recording location")
                label1.elide: Text.ElideRight
                label2.text: root.toma ? root.toma.recordingsPath : i18n("No recording location selected.")
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
                    onClicked:
                    {
                        _tomaFolderDialog.currentPath = root.toma ? root.toma.recordingsPath : FB.FM.homePath()
                        _tomaFolderDialog.callback = (paths) =>
                        {
                            if (root.toma && paths && paths.length)
                                root.toma.recordingsPath = paths[0]
                        }
                        _tomaFolderDialog.open()
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Recording format")
                label1.elide: Text.ElideRight
                label2.text: i18n("Container used for recordings.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: ["MP4", "MKV"]
                    currentIndex: root.toma && root.toma.recordingFormat === "mkv" ? 1 : 0
                    onActivated: if (root.toma) root.toma.recordingFormat = currentIndex === 1 ? "mkv" : "mp4"
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Recording quality")
                label1.elide: Text.ElideRight
                label2.text: i18n("Balance recording quality and resource usage.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: [i18n("Low"), i18n("Balanced"), i18n("High")]
                    currentIndex: root.indexForValue(["low", "balanced", "high"], root.toma ? root.toma.recordingPreset : "balanced")
                    onActivated: if (root.toma) root.toma.recordingPreset = ["low", "balanced", "high"][currentIndex]
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Recording frame rate")
                label1.elide: Text.ElideRight
                label2.text: i18n("Frames captured per second.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 7
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 7
                    from: 15
                    to: 360
                    value: root.toma ? root.toma.recordingFramerate : 60
                    onValueModified: if (root.toma) root.toma.recordingFramerate = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Recording start delay")
                label1.elide: Text.ElideRight
                label2.text: i18n("Seconds to wait before recording starts. Set to zero to start immediately.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 7
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 7
                    from: 0
                    to: 10
                    value: root.toma ? root.toma.recordingCountdown : 0
                    onValueModified: if (root.toma) root.toma.recordingCountdown = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Record audio")
                label1.elide: Text.ElideRight
                label2.text: i18n("Include audio from the selected source in recordings.")
                label2.wrapMode: Text.Wrap
                template.content: Switch
                {
                    checked: root.toma ? root.toma.audioEnabled : false
                    onToggled: if (root.toma) root.toma.audioEnabled = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Audio source")
                label1.elide: Text.ElideRight
                label2.text: i18n("Select the audio source used when recording audio.")
                label2.wrapMode: Text.Wrap
                template.content: ComboBox
                {
                    enabled: root.toma ? root.toma.audioEnabled : false
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: root.audioDeviceModel
                    textRole: "description"
                    valueRole: "name"
                    currentIndex: root.toma ? root.indexForValue(root.audioDeviceModel.map((device) => device.name), root.toma.audioDevice) : 0
                    onActivated: if (root.toma && currentIndex >= 0 && currentIndex < root.audioDeviceModel.length)
                        root.toma.audioDevice = root.audioDeviceModel[currentIndex].name
                }
            }
        }
    }
}
