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

    Maui.SectionGroup
    {
        title: i18n("User")
        description: i18n("User information")

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
            label1.text: i18n("Product")
            label2.text: manager.productName
        }

    }

    Maui.SectionGroup
    {
        title: i18n("Shell")
        description: i18n("Shell information")

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
        title: i18n("Device")
        description: i18n("Device Information")

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

        Maui.SectionItem
        {
            label1.text: i18n("Firmware")
            label2.text:  devinfo.version(DeviceInfo.Firmware)
        }

        Maui.SectionItem
        {
            label1.text: i18n("OS Version")
            label2.text:  devinfo.version(DeviceInfo.Os)
        }
    }

    Maui.SectionGroup
    {
        title: i18n("System")
        description: i18n("System Information")

        Maui.SectionItem
        {
            label1.text: i18n("Distribution")
            label2.text: manager.distroName
        }

        Maui.SectionItem
        {
            label1.text: i18n("Distribution")
            label2.text: manager.distroVersion
        }

        Maui.SectionItem
        {
            label1.text: i18n("Distribution")
            label2.text: manager.distroWebPage
        }

        Maui.SectionItem
        {
            label1.text: i18n("Arch")
            label2.text: manager.cpuArch
        }

        Maui.SectionItem
        {
            label1.text: i18n("CPU")
//            label2.text: manager.cpuInfo
        }

        Maui.SectionItem
        {
            label1.text: i18n("Kernel")
            label2.text: manager.kernelType + " / " + manager.kernelVersion
        }
    }
}
