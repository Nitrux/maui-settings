import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var info: (typeof nudgeOsdInfo !== "undefined" && nudgeOsdInfo) ? nudgeOsdInfo : null

    property var iconModeLabels: [i18n("System icons"), i18n("Emoji icons")]
    property var iconModeValues: ["system", "emoji"]

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
        text1: i18n("NudgeOSD Settings")
        text2: i18n("Configure the on-screen display.")
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
                text2: i18n("Configure the icon source and OSD dimensions.")
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Icon style")
                label2.text: i18n("Use icon-theme artwork or Nerd Font symbols.")
                template.content: ComboBox
                {
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: root.iconModeLabels
                    currentIndex: info ? root.indexForValue(root.iconModeValues, info.iconMode) : 0
                    onActivated: if (info) info.iconMode = root.iconModeValues[currentIndex]
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Width")
                label2.text: i18n("OSD width in pixels.")
                template.content: SpinBox
                {
                    from: 160
                    to: 800
                    value: info ? info.osdWidth : 292
                    onValueModified: if (info) info.osdWidth = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Height")
                label2.text: i18n("OSD height in pixels.")
                template.content: SpinBox
                {
                    from: 56
                    to: 200
                    value: info ? info.osdHeight : 66
                    onValueModified: if (info) info.osdHeight = value
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
        implicitHeight: _positionLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _positionLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Position")
                text2: i18n("Configure the OSD position on the active screen.")
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Bottom offset")
                label2.text: i18n("Distance from the bottom edge in pixels.")
                template.content: SpinBox
                {
                    from: 0
                    to: 1000
                    value: info ? info.bottomOffset : 114
                    onValueModified: if (info) info.bottomOffset = value
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
                text1: i18n("Behavior")
                text2: i18n("Configure visibility and animation timing.")
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Display duration")
                label2.text: i18n("Milliseconds before the OSD begins to hide.")
                template.content: SpinBox
                {
                    from: 250
                    to: 10000
                    stepSize: 50
                    value: info ? info.hideTimeout : 2000
                    onValueModified: if (info) info.hideTimeout = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Show animation")
                label2.text: i18n("Fade-in duration in milliseconds.")
                template.content: SpinBox
                {
                    from: 0
                    to: 2000
                    stepSize: 25
                    value: info ? info.showAnimationDuration : 200
                    onValueModified: if (info) info.showAnimationDuration = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Hide animation")
                label2.text: i18n("Fade-out duration in milliseconds.")
                template.content: SpinBox
                {
                    from: 0
                    to: 2000
                    stepSize: 25
                    value: info ? info.hideAnimationDuration : 200
                    onValueModified: if (info) info.hideAnimationDuration = value
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
        implicitHeight: _controlsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _controlsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Controls")
                text2: i18n("Set the default adjustment made by NudgeOSD commands.")
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Volume step")
                label2.text: i18n("Default percentage changed by volume shortcuts.")
                template.content: SpinBox
                {
                    from: 1
                    to: 100
                    value: info ? info.volumeStep : 5
                    onValueModified: if (info) info.volumeStep = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Brightness step")
                label2.text: i18n("Default percentage changed by brightness shortcuts.")
                template.content: SpinBox
                {
                    from: 1
                    to: 100
                    value: info ? info.brightnessStep : 10
                    onValueModified: if (info) info.brightnessStep = value
                }
            }
        }
    }
}
