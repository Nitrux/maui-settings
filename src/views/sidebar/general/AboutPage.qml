import QtQuick
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    readonly property var info: (typeof aboutInfo !== "undefined" && aboutInfo) ? aboutInfo : null

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("About")
        text2: i18n("Software, hardware, and storage details for this computer.")
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _osLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _osLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("OS")
                text2: i18n("Distribution and session details.")
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Distribution")
                label2.text: info ? info.osDistribution : i18n("Unknown")
                label2.wrapMode: Text.WordWrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Version")
                label2.text: info ? info.osVersion : i18n("Unknown")
                label2.wrapMode: Text.WordWrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Kernel")
                label2.text: info ? info.osKernel : i18n("Unknown")
                label2.wrapMode: Text.WordWrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Session")
                label2.text: info ? info.osSession : i18n("Unknown")
                label2.wrapMode: Text.WordWrap
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
        implicitHeight: _systemLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _systemLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("System")
                text2: i18n("Hardware and memory details.")
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Hostname")
                label2.text: info ? info.hostName : i18n("Unknown")
                label2.wrapMode: Text.WordWrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("CPU model")
                label2.text: info ? info.cpuModel : i18n("Unknown")
                label2.wrapMode: Text.WordWrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Architecture")
                label2.text: info ? info.cpuArchitecture : i18n("Unknown")
                label2.wrapMode: Text.WordWrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Memory total")
                label2.text: info ? info.memoryTotal : i18n("Unknown")
                label2.wrapMode: Text.WordWrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Memory available")
                label2.text: info ? info.memoryAvailable : i18n("Unknown")
                label2.wrapMode: Text.WordWrap
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
        implicitHeight: _storageLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _storageLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.medium

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Storage")
                text2: i18n("Internal partitions and filesystem details.")
            }

            Repeater
            {
                model: info ? info.storageVolumes : []

                delegate: Rectangle
                {
                    required property var modelData

                    Layout.fillWidth: true
                    color: Maui.Theme.backgroundColor
                    radius: Maui.Style.radiusV
                    border.color: Maui.Theme.alternateBackgroundColor
                    border.width: 1
                    implicitHeight: _volumeLayout.implicitHeight + Maui.Style.contentMargins * 2

                    ColumnLayout
                    {
                        id: _volumeLayout
                        anchors.fill: parent
                        anchors.margins: Maui.Style.contentMargins
                        spacing: Maui.Style.space.small

                        Maui.SectionHeader
                        {
                            Layout.fillWidth: true
                            text1: modelData.mountPoint
                            text2: [modelData.fileSystem, modelData.device].filter(Boolean).join(i18n(" · "))
                        }

                        Maui.SectionItem
                        {
                            Layout.fillWidth: true
                            flat: true
                            label1.text: i18n("Used")
                            label2.text: modelData ? modelData.used : i18n("Unknown")
                            label2.wrapMode: Text.WordWrap
                        }

                        Maui.SectionItem
                        {
                            Layout.fillWidth: true
                            flat: true
                            label1.text: i18n("Available")
                            label2.text: modelData ? modelData.available : i18n("Unknown")
                            label2.wrapMode: Text.WordWrap
                        }

                        Maui.SectionItem
                        {
                            Layout.fillWidth: true
                            flat: true
                            label1.text: i18n("Total")
                            label2.text: modelData ? modelData.total : i18n("Unknown")
                            label2.wrapMode: Text.WordWrap
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                visible: !info || info.storageVolumes.length === 0
                label1.text: i18n("No internal storage volumes were detected.")
                label2.text: ""
            }
        }
    }
}
