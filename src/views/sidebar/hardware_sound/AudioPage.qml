import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    anchors.fill: parent
    spacing: Maui.Style.space.big

    readonly property var controller: audioController
    readonly property color volumeBarColor: Maui.Theme.highlightColor
    readonly property color mutedVolumeBarColor: Maui.Theme.disabledTextColor

    function reloadSettings()
    {
        if (controller)
            controller.refresh()
    }

    function saveSettings()
    {
        return true
    }

    Component.onCompleted: reloadSettings()

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Audio")
        text2: i18n("Manage PipeWire playback, recording, and application audio.")
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: outputLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: outputLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small
            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Output Devices")
                text2: i18n("Select the default playback device and adjust its volume.")
                label2.wrapMode: Text.Wrap
            }
            Repeater
            {
                model: controller ? controller.sinks : []
                delegate: Maui.SectionItem
                {
                    property var audioObject: modelData
                    Layout.fillWidth: true
                    enabled: audioObject.available !== false
                    flat: true
                    label1.text: audioObject.description || audioObject.name
                    label1.elide: Text.ElideRight
                    label2.text: audioObject.available === false ? i18n("Unavailable output") : (audioObject.default ? i18n("Default output · %1", audioObject.volume) + "%" : i18n("Available output · %1", audioObject.volume) + "%")
                    label2.wrapMode: Text.Wrap
                    template.content: RowLayout
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
                        Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 30
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 30

                        spacing: Maui.Style.space.small
                        Slider
                        {
                            Layout.fillWidth: true
                            Layout.preferredWidth: Maui.Style.units.gridUnit * 18
                            Maui.Theme.highlightColor: audioObject.muted ? root.mutedVolumeBarColor : root.volumeBarColor
                            from: 0
                            to: 150
                            stepSize: 1
                            value: audioObject.volume
                            enabled: audioObject.hasVolume && audioObject.volumeWritable
                            onMoved: if (controller) controller.setVolume(audioObject.index, value)
                            onPressedChanged: if (!pressed && controller) controller.refresh()
                        }
                        ToolButton
                        {
                            icon.name: audioObject.muted ? "audio-volume-muted" : "audio-volume-high"
                            display: ToolButton.IconOnly
                            checkable: false
                            onClicked: if (controller) controller.setMuted(audioObject.index, !audioObject.muted)
                        }
                        Button
                        {
                            text: i18n("Default")
                            enabled: !audioObject.default
                            onClicked: if (controller) controller.setDefault(audioObject.index)
                        }
                    }
                }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: !controller || controller.sinks.length === 0
                flat: true
                label1.text: i18n("No output devices found")
                label2.text: i18n("PipeWire has not exposed a playback device.")
                label2.wrapMode: Text.Wrap
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
        implicitHeight: inputLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: inputLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Input Devices")
                text2: i18n("Select the default recording device and adjust its volume.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                model: controller ? controller.sources : []
                delegate: Maui.SectionItem
                {
                    property var audioObject: modelData
                    Layout.fillWidth: true
                    enabled: audioObject.available !== false
                    flat: true
                    label1.text: audioObject.description || audioObject.name
                    label1.elide: Text.ElideRight
                    label2.text: audioObject.available === false ? i18n("Unavailable input") : (audioObject.default ? i18n("Default input · %1", audioObject.volume) + "%" : i18n("Available input · %1", audioObject.volume) + "%")
                    label2.wrapMode: Text.Wrap

                    template.content: RowLayout
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
                        Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 30
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 30

                        spacing: Maui.Style.space.small

                        Slider
                        {
                            Layout.fillWidth: true
                            Layout.preferredWidth: Maui.Style.units.gridUnit * 18
                            Maui.Theme.highlightColor: audioObject.muted ? root.mutedVolumeBarColor : root.volumeBarColor
                            from: 0
                            to: 150
                            stepSize: 1
                            value: audioObject.volume
                            enabled: audioObject.hasVolume && audioObject.volumeWritable
                            onMoved: if (controller) controller.setVolume(audioObject.index, value)
                            onPressedChanged: if (!pressed && controller) controller.refresh()
                        }

                        ToolButton
                        {
                            icon.name: audioObject.muted ? "microphone-sensitivity-muted" : "audio-input-microphone"
                            display: ToolButton.IconOnly
                            checkable: false
                            onClicked: {
                                if (controller) controller.setSourceMuted(audioObject.index, !audioObject.muted)
                            }
                        }

                        Button
                        {
                            text: i18n("Default")
                            enabled: !audioObject.default
                            onClicked: if (controller) controller.setDefault(audioObject.index)
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: !controller || controller.sources.length === 0
                flat: true
                label1.text: i18n("No input devices found")
                label2.text: i18n("PipeWire has not exposed a recording device.")
                label2.wrapMode: Text.Wrap
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
        implicitHeight: streamsLayout.implicitHeight + Maui.Style.contentMargins * 2
        ColumnLayout
        {
            id: streamsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small
            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Application Audio")
                text2: i18n("Audio streams currently connected to PipeWire.")
                label2.wrapMode: Text.Wrap
            }
            Repeater
            {
                model: controller ? controller.playbackStreams : []
                delegate: Maui.SectionItem
                {
                    property var audioObject: modelData
                    Layout.fillWidth: true
                    flat: true
                    label1.text: audioObject.name || audioObject.description
                    label1.elide: Text.ElideRight
                    label2.text: i18n("Playback stream · %1", audioObject.volume) + "%"
                    label2.wrapMode: Text.Wrap
                    template.content: RowLayout
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
                        Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 30
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 30
                        spacing: Maui.Style.space.small
                        Slider
                        {
                            Layout.fillWidth: true
                            Layout.preferredWidth: Maui.Style.units.gridUnit * 18
                            Maui.Theme.highlightColor: audioObject.muted ? root.mutedVolumeBarColor : root.volumeBarColor
                            from: 0
                            to: 150
                            stepSize: 1
                            value: audioObject.volume
                            enabled: audioObject.hasVolume && audioObject.volumeWritable
                            onMoved: if (controller) controller.setVolume(audioObject.index, value)
                            onPressedChanged: if (!pressed && controller) controller.refresh()
                        }
                        ToolButton
                        {
                            icon.name: audioObject.muted ? "audio-volume-muted" : "audio-volume-high"
                            display: ToolButton.IconOnly
                            checkable: false
                            onClicked: if (controller) controller.setMuted(audioObject.index, !audioObject.muted)
                        }
                        Button
                        {
                            text: i18n("Default")
                            enabled: false
                            opacity: 0
                        }
                        }
                    }
                }
            Repeater
            {
                model: controller ? controller.captureStreams : []
                delegate: Maui.SectionItem
                {
                    property var audioObject: modelData
                    Layout.fillWidth: true
                    flat: true
                    label1.text: audioObject.name || audioObject.description
                    label1.elide: Text.ElideRight
                    label2.text: i18n("Recording stream · %1", audioObject.volume) + "%"
                    label2.wrapMode: Text.Wrap
                    template.content: RowLayout
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
                        Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 30
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 30
                        spacing: Maui.Style.space.small
                        Slider
                        {
                            Layout.fillWidth: true
                            Layout.preferredWidth: Maui.Style.units.gridUnit * 18
                            Maui.Theme.highlightColor: audioObject.muted ? root.mutedVolumeBarColor : root.volumeBarColor
                            from: 0
                            to: 150
                            stepSize: 1
                            value: audioObject.volume
                            enabled: audioObject.hasVolume && audioObject.volumeWritable
                            onMoved: if (controller) controller.setVolume(audioObject.index, value)
                            onPressedChanged: if (!pressed && controller) controller.refresh()
                        }
                        ToolButton
                        {
                            icon.name: audioObject.muted ? "microphone-sensitivity-muted" : "audio-input-microphone"
                            display: ToolButton.IconOnly
                            checkable: false
                            onClicked: if (controller) controller.setSourceMuted(audioObject.index, !audioObject.muted)
                        }
                        Button
                        {
                            text: i18n("Default")
                            enabled: false
                            opacity: 0
                        }
                        }
                    }
                }
            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: !controller || (controller.playbackStreams.length === 0 && controller.captureStreams.length === 0)
                flat: true
                label1.text: i18n("No active application streams")
                label2.text: i18n("Applications will appear here while they use audio.")
                label2.wrapMode: Text.Wrap
            }
        }
    }
}
