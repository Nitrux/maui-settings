import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var info: (typeof valenzInfo !== "undefined" && valenzInfo) ? valenzInfo : null

    property var screenPlacementLabels: [i18n("Active screen"), i18n("All screens")]
    property var screenPlacementValues: ["active", "all"]
    property var iconModeLabels: [i18n("System icons"), i18n("Nerd Font symbols")]
    property var iconModeValues: ["system16", "nerd"]
    property var temperatureUnitLabels: [i18n("Celsius"), i18n("Fahrenheit")]
    property var temperatureUnitValues: ["celsius", "fahrenheit"]

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
    }

    function saveSettings()
    {
        return info ? info.save() : false
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Valenz Settings")
        text2: i18n("Configure the desktop bar.")
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _barLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _barLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Desktop Bar")
            description: i18n("Control the bar size, screen placement, and layer-shell margins.")
            template.label2.wrapMode: Text.Wrap

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
                label1.text: i18n("Default spacing")
                label1.elide: Text.ElideRight
                label2.text: i18n("Fallback layer-shell margin in pixels.")
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
                    value: info ? info.barLayerSpacing : 0
                    onValueModified: if (info) info.barLayerSpacing = value
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
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _controlCenterLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _controlCenterLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Control Center")
            description: i18n("Configure the Control Center presentation and actions.")
            template.label2.wrapMode: Text.Wrap

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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 15
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 15
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 15
                    text: info ? info.controlCenterSettingsCommand : "systemsettings"
                    onEditingFinished: if (info) info.controlCenterSettingsCommand = text
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
        implicitHeight: _mediaLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _mediaLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Media")
            description: i18n("Configure the media controls shown in the bar.")
            template.label2.wrapMode: Text.Wrap

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
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _weatherLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _weatherLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Weather")
            description: i18n("Set the location and update behavior for weather information.")
            template.label2.wrapMode: Text.Wrap

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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 10
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 10
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
}
