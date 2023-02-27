// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import QtGraphicalEffects 1.0
import QtQuick.Dialogs 1.3

import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB

import org.maui.settings 1.0 as Man
import org.maui.settings.lib 1.0 as ManLib
import QtSystemInfo 5.0

ManLib.SettingsPage
{
    id: control

    property var manager : control.module.manager


    GridLayout
    {
        Layout.fillWidth: true
        columns: root.isWide ? 3 : 1
        columnSpacing: Maui.Style.space.big
        rowSpacing: Maui.Style.space.big

        Maui.SectionGroup
        {
            Layout.alignment: Qt.AlignTop
            title: i18n("User")
            description: i18n("User information")
//            template.template.iconSource: "user-identity"

            Maui.SectionItem
            {
                label1.text: i18n("Name")
                label2.text: manager.userName
            }

            Maui.SectionItem
            {
                label1.text: i18n("Machine")
                label2.text: manager.machineName
            }

            Maui.SectionItem
            {
                label1.text: i18n("Arch")
                label2.text: manager.cpuArch
            }

            Maui.SectionItem
            {
                label1.text: i18n("Kernel")
                label2.text: manager.kernelType + " / " + manager.kernelVersion
            }
        }

        Maui.SectionGroup
        {
            Layout.alignment: Qt.AlignTop

            title: i18n("Shell")
            description: i18n("Shell information")
//            template.template.iconSource: "desktop"

            Maui.SectionItem
            {
                label1.text: i18n("Desktop Name")
                label2.text: "Maui Shell"
            }

            Maui.SectionItem
            {
                label1.text: i18n("Compositor")
                label2.text: "Cask"
            }

            Maui.SectionItem
            {
                label1.text: i18n("Version")
                label2.text: "0.6.0"
            }

            Maui.SectionItem
            {
                label1.text: i18n("License")
                label2.text: "LGPL v3"
            }
        }

        Maui.SectionGroup
        {
            Layout.alignment: Qt.AlignTop

            title: i18n("Device")
            description: i18n("Device Information")
//            template.template.iconSource: "computer"

            Maui.SectionItem
            {
                label1.text: i18n("Manufacturer")
                label2.text: devinfo.manufacturer()
            }

            Maui.SectionItem
            {
                label1.text: i18n("Product")
                label2.text: devinfo.productName()
            }

            Maui.SectionItem
            {
                label1.text: i18n("Model")
                label2.text: devinfo.model()
            }

            Maui.SectionItem
            {
                label1.text: i18n("Device ID")
                label2.text: devinfo.uniqueDeviceID()
            }

//            Maui.SectionItem
//            {
//                label1.text: i18n("Firmware")
//                label2.text:  devinfo.version(DeviceInfo.Firmware)
//            }

            Maui.SectionItem
            {
                label1.text: i18n("OS Version")
                label2.text:  devinfo.version(DeviceInfo.Os)
            }
        }
    }



    Maui.SectionGroup
    {
        title: i18n("Storage")
        description: i18n("Storage device information")

        Repeater
        {
            model: manager.devices

            delegate: Maui.SectionItem
            {
                label1.text: modelData.name
                label2.text: modelData.type
                iconSource: "drive-harddisk"
                flat: false
                ProgressBar
                {
                    width: parent.parent.width
                    from: 0
                    to: modelData.bytesTotal
                    value: modelData.bytesTotal - modelData.bytesAvailable
                }
            }
        }
    }

    DeviceInfo {
        id: devinfo;
    }

    Maui.SectionGroup
    {
        title: Maui.App.about.displayName
        description: Maui.App.about.shortDescription
        //        iconSource: Maui.App.iconName

        Maui.SectionItem
        {
            label1.text: i18n("Version")
            label2.text: Maui.App.about.version
        }
        Maui.SectionItem
        {
            label1.text: i18n("Authors")
            iconSource: "view-media-artist"

            template.isMask: true
            template.iconSizeHint: Maui.Style.iconSize
            Column
            {
                spacing: Maui.Style.space.medium
                width: parent.parent.width

                Repeater
                {
                    model: Maui.App.about.authors

                    Maui.ListItemTemplate
                    {
                        id: _credits

                        width: parent.width

                        label1.text: modelData.emailAddress ? String("<a href='mailto:%1'>%2</a>").arg(modelData.emailAddress).arg(modelData.name) : modelData.name
                        label1.textFormat: Text.AutoText

                        label3.text: modelData.task

                        Connections
                        {
                            target: _credits.label1
                            function onLinkActivated(link)
                            {
                                Qt.openUrlExternally(link)
                            }
                        }
                    }
                }
            }
        }

        Maui.SectionItem
        {
            id: _translatorsSection
            label1.text: i18n("Translators")
            visible: Maui.App.about.translators.length > 0
            iconSource: "folder-language"
            template.isMask: true
            template.iconSizeHint: Maui.Style.iconSize
            Column
            {
                id: _translators
                spacing: Maui.Style.space.medium
                width: parent.parent.width

                Repeater
                {
                    model: Maui.App.about.translators

                    Maui.ListItemTemplate
                    {
                        id: _tCredits

                        width: parent.width

                        label1.text: modelData.emailAddress ? String("<a href='mailto:%1'>%2</a>").arg(modelData.emailAddress).arg(modelData.name) : modelData.name
                        label1.textFormat: Text.AutoText
                        label3.text: modelData.task
                        Connections
                        {
                            target: _tCredits.label1
                            function onLinkActivated(link)
                            {
                                Qt.openUrlExternally(link)
                            }
                        }
                    }
                }
            }
        }

        Maui.SectionItem
        {
            id: _licensesSection
            visible: Maui.App.about.licenses.length > 0
            iconSource: "license"

            template.isMask: true
            template.iconSizeHint: Maui.Style.iconSize

            label1.text: i18n("Licenses")
            // label2.text: `Maui.App.about.copyrightStatement

            Column
            {
                id: _licenses
                spacing: Maui.Style.space.medium
                width: parent.parent.width

                Repeater
                {
                    model: Maui.App.about.licenses
                    Maui.ListItemTemplate
                    {
                        width: parent.width
                        label1.text: modelData.name
                        label3.text: modelData.spdx
                    }
                }
            }
        }

        Maui.SectionItem
        {
            id: _componentsSection
            iconSource: "code-context"
            visible: Maui.App.about.components.length > 0
            template.isMask: true
            template.iconSizeHint: Maui.Style.iconSize

            label1.text: ("Components")

            Column
            {
                spacing: Maui.Style.space.medium
                width: parent.parent.width

                Repeater
                {
                    model: Maui.App.about.components
                    Maui.ListItemTemplate
                    {
                        width: parent.width
                        label1.textFormat: Text.AutoText

                        label1.text: modelData.webAddress ? String("<a href='%1'>%2</a>").arg(modelData.webAddress).arg(modelData.name) : modelData.name


                        label2.text: modelData.description
                        label3.text: modelData.version

                        Connections
                        {
                            target: label1
                            function onLinkActivated(link)
                            {
                                Qt.openUrlExternally(link)
                            }
                        }
                    }
                }
            }

        }

    }
}
