import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    anchors.fill: parent
    spacing: Maui.Style.space.big

    readonly property var controller: (typeof batteryController !== "undefined" && batteryController) ? batteryController : null
    readonly property int graphHours: graphRange.currentIndex === 0 ? 24 : 24 * 7
    readonly property int controlWidth: Maui.Style.units.gridUnit * 18

    function durationText(seconds)
    {
        if (!seconds || seconds <= 0)
            return i18n("Not available")

        const minutes = Math.max(1, Math.round(seconds / 60))
        const hours = Math.floor(minutes / 60)
        const remainingMinutes = minutes % 60
        if (hours > 0)
            return remainingMinutes > 0 ? i18n("%1 h %2 min", hours, remainingMinutes) : i18n("%1 h", hours)
        return i18n("%1 min", remainingMinutes || minutes)
    }

    function stateText()
    {
        if (!controller)
            return i18n("Unknown")

        switch (controller.state)
        {
        case "Charging":
            return i18n("Charging")
        case "Discharging":
            return i18n("Discharging")
        case "Fully charged":
            return i18n("Fully charged")
        case "Empty":
            return i18n("Empty")
        default:
            return i18n("Unknown")
        }
    }

    function visibleHistory()
    {
        if (!controller)
            return []

        const cutoff = Date.now() - graphHours * 60 * 60 * 1000
        return controller.history.filter((entry) => entry.timestamp >= cutoff)
    }

    Component.onCompleted: if (controller) controller.refresh()

    Timer
    {
        interval: 60 * 1000
        repeat: true
        running: root.visible
        onTriggered: if (root.controller) root.controller.refresh()
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Battery")
        text2: i18n("Monitor charge, battery health, and system power information.")
        label2.wrapMode: Text.Wrap
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: overviewLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: overviewLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Charge Level")
                text2: i18n("Battery charge level and estimated time remaining.")
                label2.wrapMode: Text.Wrap
            }

            Rectangle
            {
                Layout.fillWidth: true
                color: Maui.Theme.backgroundColor
                radius: Maui.Style.radiusV
                border.color: Maui.Theme.alternateBackgroundColor
                border.width: 1
                implicitHeight: chargingCardLayout.implicitHeight + Maui.Style.contentMargins * 2

                ColumnLayout
                {
                    id: chargingCardLayout
                    anchors.fill: parent
                    anchors.margins: Maui.Style.contentMargins
                    spacing: Maui.Style.space.small

                    RowLayout
                    {
                        Layout.fillWidth: true
                        spacing: Maui.Style.space.medium

                        Maui.SectionHeader
                        {
                            Layout.fillWidth: true
                            text1: root.controller && root.controller.hasBattery
                                   ? root.stateText()
                                   : root.controller && !root.controller.available
                                     ? i18n("UPower is not available")
                                     : i18n("No battery detected")
                            text2: root.controller && root.controller.hasBattery
                                   ? root.controller.charging
                                     ? i18n("%1 until fully charged", root.durationText(root.controller.timeToFull))
                                     : i18n("%1 remaining", root.durationText(root.controller.timeToEmpty))
                                   : i18n("Connect a battery to see its status.")
                            label1.elide: Text.ElideRight
                            label2.wrapMode: Text.Wrap
                        }

                        Label
                        {
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            text: root.controller && root.controller.hasBattery ? root.controller.percentage + "%" : "—"
                            font.pointSize: Maui.Style.fontSizes.enormous
                            font.weight: Font.Medium
                        }
                    }

                    ProgressBar
                    {
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        value: root.controller && root.controller.hasBattery ? root.controller.percentage : 0
                    }

                    Text
                    {
                        Layout.fillWidth: true
                        Layout.leftMargin: Maui.Style.defaultPadding
                        Layout.rightMargin: Maui.Style.defaultPadding
                        text: {
                            if (!root.controller || !root.controller.hasBattery)
                                return i18n("Power source is not available.")

                            const source = root.controller.onBattery ? i18n("Battery") : i18n("AC adapter")
                            if (root.controller.energyRate > 0)
                                return i18n("Power source: %1 · Current draw: %2 W", source, root.controller.energyRate.toFixed(1))
                            return i18n("Power source: %1", source)
                        }
                        color: Maui.Theme.textColor
                        opacity: 0.75
                        wrapMode: Text.Wrap
                        font.pointSize: Maui.Style.fontSizes.small
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
        implicitHeight: historyLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: historyLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Battery History")
                text2: i18n("Aggregate charge level and discharge rate history.")
                label2.wrapMode: Text.Wrap
            }

            ComboBox
            {
                id: graphRange
                Layout.alignment: Qt.AlignRight
                Layout.minimumWidth: root.controlWidth
                Layout.preferredWidth: root.controlWidth
                Layout.maximumWidth: root.controlWidth
                model: [i18n("Last 24 hours"), i18n("Last 7 days")]
            }

            Item
            {
                id: graphItem
                Layout.fillWidth: true
                implicitHeight: 220

                Canvas
                {
                    id: graph
                    anchors.fill: parent
                    property var samples: root.visibleHistory()
                    property color gridColor: Qt.rgba(Maui.Theme.textColor.r, Maui.Theme.textColor.g, Maui.Theme.textColor.b, 0.18)
                    property color dischargeColor: Maui.Theme.negativeTextColor

                    onSamplesChanged: requestPaint()
                    onWidthChanged: requestPaint()
                    onHeightChanged: requestPaint()

                    onPaint:
                    {
                        const context = getContext("2d")
                        context.reset()

                        const left = 42
                        const top = 12
                        const right = 10
                        const bottom = 28
                        const width = Math.max(1, graph.width - left - right)
                        const height = Math.max(1, graph.height - top - bottom)

                        context.font = "12px sans-serif"
                        context.textAlign = "right"
                        context.textBaseline = "middle"
                        context.strokeStyle = graph.gridColor
                        context.fillStyle = Maui.Theme.textColor
                        context.lineWidth = 1

                        for (let i = 0; i <= 4; ++i)
                        {
                            const value = 100 - i * 25
                            const y = top + i * height / 4
                            context.beginPath()
                            context.moveTo(left, y)
                            context.lineTo(left + width, y)
                            context.stroke()
                            context.fillText(value + "%", left - 8, y)
                        }

                        const samples = graph.samples || []
                        if (!samples.length)
                            return

                        const now = Date.now()
                        const start = now - root.graphHours * 60 * 60 * 1000
                        const point = (entry) => {
                            const x = left + Math.max(0, Math.min(1, (entry.timestamp - start) / (now - start))) * width
                            const y = top + (100 - entry.percentage) / 100 * height
                            return Qt.point(x, y)
                        }

                        context.beginPath()
                        const first = point(samples[0])
                        context.moveTo(first.x, top + height)
                        context.lineTo(first.x, first.y)
                        for (let i = 1; i < samples.length; ++i)
                        {
                            const next = point(samples[i])
                            context.lineTo(next.x, next.y)
                        }
                        const last = point(samples[samples.length - 1])
                        context.lineTo(last.x, top + height)
                        context.closePath()
                        context.fillStyle = Qt.rgba(Maui.Theme.highlightColor.r, Maui.Theme.highlightColor.g, Maui.Theme.highlightColor.b, 0.18)
                        context.fill()

                        context.beginPath()
                        context.moveTo(first.x, first.y)
                        for (let i = 1; i < samples.length; ++i)
                        {
                            const next = point(samples[i])
                            context.lineTo(next.x, next.y)
                        }
                        context.strokeStyle = Maui.Theme.highlightColor
                        context.lineWidth = 3
                        context.lineJoin = "round"
                        context.lineCap = "round"
                        context.stroke()

                        let measuredMaximumDischargeRate = 0
                        for (let i = 0; i < samples.length; ++i)
                            measuredMaximumDischargeRate = Math.max(measuredMaximumDischargeRate, Number(samples[i].dischargeRate) || 0)

                        const maximumDischargeRate = Math.max(1, measuredMaximumDischargeRate)
                        const ratePoint = (entry) => {
                            const x = left + Math.max(0, Math.min(1, (entry.timestamp - start) / (now - start))) * width
                            const rate = Math.max(0, Number(entry.dischargeRate) || 0)
                            const y = top + (1 - Math.min(1, rate / maximumDischargeRate)) * height
                            return Qt.point(x, y)
                        }

                        if (measuredMaximumDischargeRate > 0)
                        {
                            context.beginPath()
                            let rateStarted = false
                            for (let i = 0; i < samples.length; ++i)
                            {
                                if ((Number(samples[i].dischargeRate) || 0) <= 0)
                                {
                                    rateStarted = false
                                    continue
                                }

                                const next = ratePoint(samples[i])
                                if (!rateStarted)
                                {
                                    context.moveTo(next.x, next.y)
                                    rateStarted = true
                                }
                                else
                                    context.lineTo(next.x, next.y)
                            }
                            context.strokeStyle = graph.dischargeColor
                            context.lineWidth = 2
                            context.lineJoin = "round"
                            context.lineCap = "round"
                            context.stroke()

                            context.font = "12px sans-serif"
                            context.textAlign = "left"
                            context.textBaseline = "middle"
                            context.fillStyle = graph.dischargeColor
                            context.fillText(maximumDischargeRate.toFixed(1) + " W", left + width + 8, top)
                            context.fillText("0 W", left + width + 8, top + height)
                        }
                    }
                }

                Label
                {
                    anchors.centerIn: parent
                    visible: !graph.samples || graph.samples.length === 0
                    text: i18n("History will appear after the first sample.")
                    opacity: 0.7
                }
            }

            RowLayout
            {
                Layout.fillWidth: true
                Layout.leftMargin: Maui.Style.defaultPadding
                Layout.rightMargin: Maui.Style.defaultPadding
                spacing: Maui.Style.space.medium

                RowLayout
                {
                    spacing: Maui.Style.space.small

                    Rectangle
                    {
                        width: Maui.Style.units.gridUnit * 0.5
                        height: width
                        radius: width / 2
                        color: Maui.Theme.highlightColor
                    }

                    Label
                    {
                        text: i18n("Charge level")
                        opacity: 0.75
                    }
                }

                RowLayout
                {
                    spacing: Maui.Style.space.small

                    Rectangle
                    {
                        width: Maui.Style.units.gridUnit * 0.5
                        height: width
                        radius: width / 2
                        color: Maui.Theme.negativeTextColor
                    }

                    Label
                    {
                        text: i18n("Discharge rate")
                        opacity: 0.75
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
        implicitHeight: detailsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: detailsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Battery Details")
                text2: i18n("Technical information about the battery.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Battery health")
                label1.elide: Text.ElideRight
                label2.text: root.controller && root.controller.hasBattery && root.controller.health > 0
                              ? root.controller.capacityLevel.length > 0
                                ? i18n("%1% · %2", root.controller.health, root.controller.capacityLevel)
                                : i18n("%1%", root.controller.health)
                              : i18n("N/A")
                label2.wrapMode: Text.Wrap
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Energy")
                label1.elide: Text.ElideRight
                label2.text: root.controller && root.controller.hasBattery && root.controller.energyFull > 0
                              ? root.controller.energyFullDesign > 0
                                ? i18n("%1 Wh of %2 Wh · %3 Wh design", root.controller.energy.toFixed(1), root.controller.energyFull.toFixed(1), root.controller.energyFullDesign.toFixed(1))
                                : i18n("%1 Wh of %2 Wh", root.controller.energy.toFixed(1), root.controller.energyFull.toFixed(1))
                              : i18n("N/A")
                label2.wrapMode: Text.Wrap
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Voltage")
                label1.elide: Text.ElideRight
                label2.text: root.controller && root.controller.voltage > 0
                              ? i18n("%1 V", root.controller.voltage.toFixed(3))
                              : i18n("N/A")
                label2.wrapMode: Text.Wrap
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Technology")
                label1.elide: Text.ElideRight
                label2.text: root.controller && root.controller.technology.length > 0
                              ? root.controller.technology
                              : i18n("N/A")
                label2.wrapMode: Text.Wrap
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Charge cycles")
                label1.elide: Text.ElideRight
                label2.text: root.controller && root.controller.chargeCycles >= 0
                              ? String(root.controller.chargeCycles)
                              : i18n("N/A")
                label2.wrapMode: Text.Wrap
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Battery")
                label1.elide: Text.ElideRight
                label2.text: root.controller && root.controller.hasBattery
                              ? [root.controller.vendor, root.controller.model].filter((value) => value.length > 0).join(" ") || i18n("Unknown battery")
                              : i18n("Not available")
                label2.wrapMode: Text.Wrap
            }
        }
    }
}
