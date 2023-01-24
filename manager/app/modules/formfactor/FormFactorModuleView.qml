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
        title: i18n("Options")
        description: i18n("Optional configurations")

        Maui.SettingTemplate
        {
            label1.text: i18n("Preferred Mode")
            label2.text: i18n("Switch between the modes manually.")

            Maui.ToolActions
            {
                autoExclusive: true
                checkable: true

                Action
                {
                    text: i18n("Mobile")
                    icon.name: "phone"

                    checked: control.manager.preferredMode === 2
                    onTriggered:  control.manager.preferredMode = 2
                }

                Action
                {
                    text: i18n("Tablet")
                    icon.name: "tablet"
                    checked: control.manager.preferredMode === 1
                    onTriggered:  control.manager.preferredMode = 1


                }

                Action
                {
                    text: i18n("Desktop")
                    icon.name: "computer-laptop"

                    checked: control.manager.preferredMode === 0
                    onTriggered:  control.manager.preferredMode = 0

                }
            }
        }

    }

    Maui.SettingsSection
    {
        title: i18n("Information")
        description: i18n("Form factor information based on inputs.")


        GridLayout
        {
            //                        Layout.margins: Maui.Style.space.medium
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignCenter
            columnSpacing: Maui.Style.space.big
            rowSpacing: columnSpacing
            columns: 2

            Maui.ListBrowserDelegate
            {
                checked: control.manager.hasKeyboard
                enabled: control.manager.hasKeyboard
                checkable: false
                flat: false
                Layout.fillWidth: true
                iconSizeHint: Maui.Style.iconSizes.big
                template.isMask: false
                iconSource: "input-keyboard"
                label1.text: i18n("Keyboard")
                label2.text: i18n("The device has a physical keyboard")
                label2.wrapMode: Text.Wrap
            }

            Maui.ListBrowserDelegate
            {
                checked: control.manager.hasMouse
                enabled: checked
                checkable: false
                flat: false
                Layout.fillWidth: true
                iconSizeHint: Maui.Style.iconSizes.big
                template.isMask: false
                iconSource:"input-mouse"
                label1.text: i18n("Mouse")
            }

            Maui.ListBrowserDelegate
            {
                checked: control.manager.hasTouchpad
                enabled: checked
                checkable: false
                flat: false
                Layout.fillWidth: true
                iconSizeHint: Maui.Style.iconSizes.big
                template.isMask: false

                iconSource:"input-touchpad"
                label1.text: i18n("Trackpad")
            }

            Maui.ListBrowserDelegate
            {
                checked: control.manager.hasTouchscreen
                enabled: checked
                checkable: false
                flat: false
                Layout.fillWidth: true
                iconSizeHint: Maui.Style.iconSizes.big
                template.isMask: false
                iconSource:"hand"
                label1.text: i18n("Touch")
                label2.text: i18n("The device has a touch screen.")

            }
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Best Mode")
            label2.text: i18n("Given the inputs this is the best mode.")

            Maui.ToolActions
            {
                autoExclusive: false
                checkable: false

                Action
                {
                    text: i18n("Mobile")
                    checked: control.manager.bestMode === 2
                }

                Action
                {
                    text: i18n("Tablet")
                    checked: control.manager.bestMode === 1

                }

                Action
                {
                    text: i18n("Desktop")
                    checked: control.manager.bestMode === 0
                }
            }
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Deafult Mode")
            label2.text: i18n("This is the default mode intended by you distro.")

            Maui.ToolActions
            {
                autoExclusive: true
                checkable: false

                Action
                {
                    text: i18n("Mobile")
                    checked: control.manager.defaultMode === 2
                }

                Action
                {
                    text: i18n("Tablet")
                    checked: control.manager.defaultMode === 1

                }

                Action
                {
                    text: i18n("Desktop")
                    checked: control.manager.defaultMode === 0
                }
            }
        }
    }

}
