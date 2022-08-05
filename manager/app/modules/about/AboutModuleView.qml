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

ManLib.SettingsPage
{
    id: control

    property var module
    property var manager : control.module.manager
    title: module.name

    Maui.SettingsSection
    {
        title: i18n("User")
        description: i18n("User information")

        Maui.SettingTemplate
        {
            label1.text: i18n("Name")
            label2.text: manager.userName

        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Machine")
            label2.text: manager.machineName

        }


        Maui.SettingTemplate
        {
            label1.text: i18n("Product")
            label2.text: manager.productName
        }

    }

    Maui.SettingsSection
    {
        title: i18n("Storage")
        description: i18n("Storage device information")

        Repeater
        {
            model: manager.devices

            delegate: Maui.SettingTemplate
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

    Maui.SettingsSection
    {
        title: i18n("System")
        description: i18n("System Information")

        Maui.SettingTemplate
        {
            label1.text: i18n("Distribution")
            label2.text: manager.distroName
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Distribution")
            label2.text: manager.distroVersion
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Distribution")
            label2.text: manager.distroWebPage
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Arch")
            label2.text: manager.cpuArch
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("CPU")
            label2.text: manager.cpuInfo
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Kernel")
            label2.text: manager.kernelType + " / " + manager.kernelVersion
        }
    }

}
