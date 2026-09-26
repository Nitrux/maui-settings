import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.mauikit.controls as Maui

Maui.ScrollColumn {
    id: root
    anchors.fill: parent
    spacing: Maui.Style.space.big
    readonly property var info: powerInfo
    readonly property var graphics: (typeof graphicsModeController !== "undefined" && graphicsModeController) ? graphicsModeController : null
    readonly property var graphicsModes: [
        { mode: "integrated", label: i18n("Integrated"), description: i18n("Use integrated graphics to reduce power consumption. External displays connected to the Nvidia GPU may be unavailable.") },
        { mode: "hybrid", label: i18n("Hybrid"), description: i18n("Use integrated graphics by default and enable Nvidia rendering when needed.") },
        { mode: "nvidia", label: i18n("Nvidia"), description: i18n("Use the Nvidia GPU exclusively for higher graphics performance and power consumption.") }
    ]
    property string requestedGraphicsMode: ""
    function graphicsModeIndex(mode) {
        for (let i = 0; i < graphicsModes.length; ++i)
            if (graphicsModes[i].mode === mode)
                return i
        return -1
    }
    function graphicsModeInfo(mode) {
        for (const item of graphicsModes)
            if (item.mode === mode)
                return item
        return { label: i18n("Unknown"), description: i18n("The current graphics mode is not available.") }
    }
    readonly property string displayedGraphicsMode: graphics && graphics.rebootRequired && graphics.pendingMode
        ? graphics.pendingMode
        : graphics ? graphics.currentMode : ""
    function syncGraphicsModeSelection() {
        if (graphicsModeCombo)
            graphicsModeCombo.currentIndex = graphics && graphics.available
                ? graphicsModeIndex(displayedGraphicsMode)
                : 0
    }
    function requestGraphicsMode(mode) {
        if (!graphics || graphics.busy || !mode || (mode === graphics.currentMode && !graphics.rebootRequired))
            return
        requestedGraphicsMode = mode
        graphicsModeDialog.open()
    }
    readonly property bool daemonReady: info && info.daemonAvailable && info.daemonRunning && info.configAvailable
    function reloadSettings() { if (info) info.reload() }
    function saveSettings() { return info ? info.save() : false }
    function responsive(control) {
        control.responsiveSectionItem = control.parent.parent.parent.parent
        control.wideParent = control.parent
        control.updateResponsiveParent()
    }
    Component.onCompleted: { reloadSettings(); syncGraphicsModeSelection() }

    Connections
    {
        target: root.graphics
        function onStateChanged() { root.syncGraphicsModeSelection() }
    }

    Maui.SectionHeader {
        Layout.fillWidth: true
        text1: i18n("Nitrux Performance Tuning")
        text2: i18n("Configure thresholds that affect the system's performance.")
        label2.wrapMode: Text.Wrap
    }

    Rectangle {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: settingsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout {
            id: settingsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            enabled: root.daemonReady
            spacing: Maui.Style.space.small

            Maui.SectionHeader {
                Layout.fillWidth: true
                text1: i18n("NX Power Daemon")
                text2: !info || !info.daemonAvailable ? i18n("nx-powerd is not installed.")
                       : !info.daemonRunning ? i18n("nx-powerd is not running.")
                       : !info.configAvailable ? i18n("The nx-powerd configuration file was not found.")
                       : i18n("Enable automatic profile changes while keeping battery notifications active.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable Automatic mode")
                label1.elide: Text.ElideRight
                label2.text: i18n("Turn off to select power profiles manually.")
                label2.wrapMode: Text.Wrap
                template.content: Switch {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : implicitWidth
                    checked: info ? info.enabled : true
                    onToggled:
                    {
                        if (!info)
                            return

                        info.enabled = checked
                        info.save()
                    }
                }
            }

            Maui.SectionItem {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Power saver at or below")
                label1.elide: Text.ElideRight
                label2.text: i18n("Battery percentage at which power saver is selected.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    from: 0; to: 98
                    value: info ? info.powerSaverMax : 20
                    onValueModified: if (info) info.powerSaverMax = value
                }
            }

            Maui.SectionItem {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Balanced up to")
                label1.elide: Text.ElideRight
                label2.text: i18n("Battery percentage below which balanced remains selected.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    from: info ? info.powerSaverMax + 1 : 21; to: 99
                    value: info ? info.balancedMax : 59
                    onValueModified: if (info) info.balancedMax = value
                }
            }

            Maui.SectionItem {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Performance at or above")
                label1.elide: Text.ElideRight
                label2.text: i18n("Battery percentage at which performance is selected.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    from: info ? info.balancedMax + 1 : 60; to: 100
                    value: info ? info.performanceMin : 60
                    onValueModified: if (info) info.performanceMin = value
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
        implicitHeight: graphicsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: graphicsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Graphics Mode")
                text2: !root.graphics || !root.graphics.available
                    ? i18n("Graphics mode switching is unavailable on this system.")
                    : root.graphics.rebootRequired
                    ? i18n("%1 is selected and will be applied after restarting.", root.graphicsModeInfo(root.graphics.pendingMode).label)
                    : i18n("Select which GPU is used when the system starts.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: !root.graphics || !root.graphics.available
                flat: true
                label1.text: i18n("Graphics mode unavailable")
                label1.elide: Text.ElideRight
                label2.text: i18n("No Optimus-compatible graphics hardware was found, or the required graphics tools are not installed.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: !!(root.graphics && root.graphics.available)
                flat: true
                enabled: !!(root.graphics && root.graphics.available && !root.graphics.busy)
                label1.text: i18n("Mode")
                label1.elide: Text.ElideRight
                label2.text: root.graphics && root.graphics.available
                    ? root.graphicsModeInfo(root.displayedGraphicsMode).description
                    : i18n("Select a mode when Optimus-compatible graphics hardware is available.")
                label2.wrapMode: Text.Wrap

                template.content: ComboBox
                {
                    id: graphicsModeCombo
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    enabled: !!(root.graphics && root.graphics.available && !root.graphics.busy)
                    model: root.graphics && root.graphics.available
                        ? root.graphicsModes
                        : [{ mode: "", label: i18n("Unavailable"), description: "" }]
                    textRole: "label"
                    currentIndex: root.graphics && root.graphics.available
                        ? root.graphicsModeIndex(root.displayedGraphicsMode)
                        : 0
                    onActivated: if (root.graphics && root.graphics.available && currentIndex >= 0)
                        root.requestGraphicsMode(root.graphicsModes[currentIndex].mode)
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: root.graphics && root.graphics.rebootRequired
                flat: true
                label1.text: i18n("Restart required")
                label1.elide: Text.ElideRight
                label2.text: i18n("Restart the system to apply the selected graphics mode.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: root.graphics && root.graphics.errorMessage.length > 0
                flat: true
                label1.text: i18n("Graphics mode error")
                label1.elide: Text.ElideRight
                label2.text: root.graphics ? root.graphics.errorMessage : ""
                label2.wrapMode: Text.Wrap
            }
        }
    }

    Maui.SettingsDialog
    {
        id: graphicsModeDialog
        title: i18n("Change Graphics Mode")
        persistent: true
        onClosed: root.syncGraphicsModeSelection()

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: root.graphicsModeInfo(root.requestedGraphicsMode).label
                text2: root.graphicsModeInfo(root.requestedGraphicsMode).description
                label2.wrapMode: Text.Wrap
            }

            Label
            {
                Layout.fillWidth: true
                text: i18n("Changing the graphics mode modifies system startup configuration and requires a restart. Continue?")
                wrapMode: Text.Wrap
            }
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: graphicsModeDialog.close()
            },
            Action
            {
                text: i18n("Apply")
                enabled: root.graphics && !root.graphics.busy && root.requestedGraphicsMode.length > 0
                onTriggered:
                {
                    root.graphics.setMode(root.requestedGraphicsMode)
                    graphicsModeDialog.close()
                }
            }
        ]
    }
}
