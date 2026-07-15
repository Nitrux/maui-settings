import QtQuick
import QtQuick.Controls
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
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Storage")
                text2: i18n("Physical disks and their storage usage.")
            }

            Repeater
            {
                model: info ? info.storageDevices : []

                delegate: Rectangle
                {
                    required property var modelData

                    Layout.fillWidth: true
                    color: Maui.Theme.backgroundColor
                    radius: Maui.Style.radiusV
                    border.color: Maui.Theme.alternateBackgroundColor
                    border.width: 1
                    implicitHeight: _deviceLayout.implicitHeight + Maui.Style.contentMargins * 2

                    ColumnLayout
                    {
                        id: _deviceLayout
                        anchors.fill: parent
                        anchors.margins: Maui.Style.contentMargins
                        spacing: Maui.Style.space.small

                        Maui.SectionHeader
                        {
                            Layout.fillWidth: true
                            text1: modelData.title
                            text2: modelData.subtitle
                        }

                        ProgressBar
                        {
                            Layout.fillWidth: true
                            Layout.leftMargin: Maui.Style.defaultPadding
                            Layout.rightMargin: Maui.Style.defaultPadding
                            from: 0
                            to: 1
                            value: modelData.usageFraction >= 0 ? modelData.usageFraction : 0
                            indeterminate: modelData.indeterminate
                        }

                        Text
                        {
                            Layout.fillWidth: true
                            Layout.leftMargin: Maui.Style.defaultPadding
                            Layout.rightMargin: Maui.Style.defaultPadding
                            text: modelData.usageSummary
                            color: Maui.Theme.textColor
                            opacity: 0.75
                            wrapMode: Text.WordWrap
                            font.pointSize: Maui.Style.fontSizes.small
                        }
                    }
                }
            }
        }
    }
}
