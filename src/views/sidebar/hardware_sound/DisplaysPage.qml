import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    anchors.fill: parent
    spacing: Maui.Style.space.big

    readonly property var controller: (typeof displaysController !== "undefined" && displaysController) ? displaysController : null
    readonly property int controlWidth: Maui.Style.units.gridUnit * 13
    readonly property int spinnerWidth: Maui.Style.units.gridUnit * 7
    readonly property real previewPadding: 18
    readonly property real previewSpacing: Maui.Style.space.small
    readonly property real layoutMinX: {
        let value = 0
        if (controller)
            for (const monitor of controller.monitors)
                value = Math.min(value, Number(monitor.x) || 0)
        return value
    }
    readonly property real layoutMinY: {
        let value = 0
        if (controller)
            for (const monitor of controller.monitors)
                value = Math.min(value, Number(monitor.y) || 0)
        return value
    }
    readonly property real layoutMaxX: {
        let value = 1
        if (controller)
            for (const monitor of controller.monitors)
                value = Math.max(value, (Number(monitor.x) || 0) + Math.max(1, Number(monitor.width) || 1))
        return value
    }
    readonly property real layoutMaxY: {
        let value = 1
        if (controller)
            for (const monitor of controller.monitors)
                value = Math.max(value, (Number(monitor.y) || 0) + Math.max(1, Number(monitor.height) || 1))
        return value
    }
    readonly property real layoutWidth: Math.max(1, layoutMaxX - layoutMinX)
    readonly property real layoutHeight: Math.max(1, layoutMaxY - layoutMinY)

    function reloadSettings()
    {
        if (controller)
            controller.reload()
    }

    function saveSettings()
    {
        return controller ? controller.save() : false
    }

    function responsive(control)
    {
        control.responsiveSectionItem = control.parent.parent.parent
        control.wideParent = control.parent
        control.updateResponsiveParent()
    }

    Component.onCompleted: reloadSettings()

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Displays")
        text2: !root.controller || !root.controller.available
            ? i18n("hyprscreend and Hyprland display information are not available.")
            : i18n("Choose automatic display management or configure each monitor manually.")
        label2.wrapMode: Text.Wrap
    }

    Rectangle
    {
        id: preview
        Layout.fillWidth: true
        implicitHeight: 230
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        clip: true

        readonly property real previewContentTop: arrangementHeader.y + arrangementHeader.height + root.previewSpacing
        readonly property real previewContentHeight: Math.max(1, height - previewContentTop - root.previewPadding)
        property real previewScale: Math.min(
            (width - root.previewPadding * 2) / root.layoutWidth,
            previewContentHeight / root.layoutHeight)
        readonly property real layoutOffsetX: root.previewPadding
            + Math.max(0, (width - root.previewPadding * 2 - root.layoutWidth * previewScale) / 2)
        readonly property real layoutOffsetY: previewContentTop
            + Math.max(0, (previewContentHeight - root.layoutHeight * previewScale) / 2)

        Maui.SectionHeader
        {
            id: arrangementHeader
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: Maui.Style.contentMargins
            text1: i18n("Display Arrangement")
            text2: i18n("A simple view of the connected monitor positions.")
            label2.wrapMode: Text.Wrap
        }

        Repeater
        {
            model: root.controller ? root.controller.monitors : []

            delegate: Rectangle
            {
                required property var modelData
                required property int index
                readonly property bool selected: root.controller && index === root.controller.selectedMonitorIndex
                x: preview.layoutOffsetX + (Number(modelData.x) - root.layoutMinX) * preview.previewScale
                y: preview.layoutOffsetY + (Number(modelData.y) - root.layoutMinY) * preview.previewScale
                width: Math.max(1, (Number(modelData.width) || 160) * preview.previewScale)
                height: Math.max(1, (Number(modelData.height) || 90) * preview.previewScale)
                color: selected ? Maui.Theme.highlightColor : Maui.Theme.backgroundColor
                border.color: selected ? Maui.Theme.highlightedTextColor : Maui.Theme.textColor
                border.width: selected ? 2 : 1
                radius: Maui.Style.radiusV

                Label
                {
                    anchors.centerIn: parent
                    width: parent.width - Maui.Style.defaultPadding * 2
                    text: modelData.name
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    color: selected ? Maui.Theme.highlightedTextColor : Maui.Theme.textColor
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked: if (root.controller) root.controller.selectMonitor(index)
                }
            }
        }

        Label
        {
            anchors.centerIn: parent
            visible: !root.controller || root.controller.monitors.length === 0
            text: i18n("No connected monitors detected.")
            opacity: 0.7
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
        implicitHeight: displayLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: displayLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Display Management")
                text2: root.controller && !root.controller.configAvailable
                    ? i18n("The hyprscreend configuration will be created when you save these settings.")
                    : root.controller && root.controller.automatic
                      ? i18n("Automatic mode lets hyprscreend manage monitor modes and scaling.")
                      : i18n("Manual mode writes mode, scale, refresh rate, and position to the Hyprland monitor blocks.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable automatic display management")
                label1.elide: Text.ElideRight
                label2.text: i18n("Turn off to let this module configure connected monitors.")
                label2.wrapMode: Text.Wrap
                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : implicitWidth
                    checked: root.controller ? root.controller.automatic : true
                    onToggled: if (root.controller) root.controller.automatic = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Display")
                label1.elide: Text.ElideRight
                label2.text: root.controller && root.controller.monitors.length > 0
                    ? i18n("Internal and external monitors can be configured here.")
                    : i18n("No connected monitor is available to configure.")
                label2.wrapMode: Text.Wrap
                template.content: ComboBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.controlWidth
                    Layout.preferredWidth: root.controlWidth
                    model: root.controller ? root.controller.monitors : []
                    textRole: "display"
                    currentIndex: root.controller ? root.controller.selectedMonitorIndex : 0
                    onActivated: if (root.controller) root.controller.selectMonitor(currentIndex)
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: root.controller && !root.controller.automatic
                label1.text: i18n("Scaling")
                label1.elide: Text.ElideRight
                label2.text: i18n("Set the display scale percentage used by Hyprland.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.spinnerWidth
                    Layout.preferredWidth: root.spinnerWidth
                    from: 50
                    to: 300
                    stepSize: 5
                    value: root.controller ? root.controller.scalePercent : 100
                    onValueModified: if (root.controller) root.controller.scalePercent = value
                    valueFromText: function(text, locale) { return Number(text.replace("%", "")) }
                    textFromValue: function(value) { return value + "%" }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: root.controller && !root.controller.automatic
                label1.text: i18n("Resolution")
                label1.elide: Text.ElideRight
                label2.text: i18n("Choose an available resolution for the selected display.")
                label2.wrapMode: Text.Wrap
                template.content: ComboBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.controlWidth
                    Layout.preferredWidth: root.controlWidth
                    model: root.controller ? root.controller.resolutions : []
                    currentIndex: root.controller ? model.indexOf(root.controller.selectedResolution) : 0
                    onActivated: if (root.controller) root.controller.selectedResolution = currentText
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: root.controller && !root.controller.automatic
                label1.text: i18n("Refresh rate")
                label1.elide: Text.ElideRight
                label2.text: i18n("Choose an available refresh rate for the selected resolution.")
                label2.wrapMode: Text.Wrap
                template.content: ComboBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.controlWidth
                    Layout.preferredWidth: root.controlWidth
                    model: root.controller ? root.controller.refreshRates : []
                    currentIndex: root.controller ? model.indexOf(Number(root.controller.selectedRefreshRate).toFixed(root.controller.selectedRefreshRate % 1 === 0 ? 0 : 2)) : 0
                    onActivated: if (root.controller) root.controller.selectedRefreshRate = Number(currentText)
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: root.controller && !root.controller.automatic
                label1.text: i18n("Position X")
                label1.elide: Text.ElideRight
                label2.text: i18n("Set the horizontal position in the monitor layout.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.spinnerWidth
                    Layout.preferredWidth: root.spinnerWidth
                    from: -32768
                    to: 32768
                    value: root.controller ? root.controller.positionX : 0
                    onValueModified: if (root.controller) root.controller.positionX = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: root.controller && !root.controller.automatic
                label1.text: i18n("Position Y")
                label1.elide: Text.ElideRight
                label2.text: i18n("Set the vertical position in the monitor layout.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.spinnerWidth
                    Layout.preferredWidth: root.spinnerWidth
                    from: -32768
                    to: 32768
                    value: root.controller ? root.controller.positionY : 0
                    onValueModified: if (root.controller) root.controller.positionY = value
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        visible: root.controller && root.controller.hasUnusedEntries
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: cleanupLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: cleanupLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Unused Monitors")
                text2: i18n("These monitor blocks can be removed from the Hyprland configuration.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                model: root.controller ? root.controller.unusedEntries : []

                delegate: Maui.SectionItem
                {
                    required property var modelData
                    Layout.fillWidth: true
                    flat: true
                    label1.text: modelData.output
                    label1.elide: Text.ElideRight
                    label2.text: modelData.mode ? i18n("Configured mode: %1", modelData.mode) : i18n("Configured monitor entry")
                    label2.wrapMode: Text.Wrap
                    template.content: ToolButton
                    {
                        icon.name: "edit-delete"
                        display: ToolButton.IconOnly
                        ToolTip.visible: hovered
                        ToolTip.text: i18n("Remove monitor")
                        onClicked: if (root.controller) root.controller.removeUnusedEntry(modelData.output)
                    }
                }
            }
        }
    }
}
