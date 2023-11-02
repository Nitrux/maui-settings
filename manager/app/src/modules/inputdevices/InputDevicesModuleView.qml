// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.maui.settings.lib as ManLib

ManLib.SettingsPage
{
    id: control

    property var manager : control.module.manager
    Component
    {
        id: _keyboardPageComponent
        Maui.SettingsPage
        {
            title: i18n("Keymap")
            Maui.SectionItem
            {
                label1.text: i18n("Layout")
                label2.text: i18n("Keyboard layout.")
                columns: 1

                TextField
                {
                    Layout.fillWidth: true
                    text: control.manager.keyboardLayout
                    onAccepted: control.manager.keyboardLayout = text
                }
            }

            Maui.SectionItem
            {
                label1.text: i18n("Model")
                label2.text: i18n("Keyboard model.")
                columns: 1

                TextField
                {
                    Layout.fillWidth: true
                    text: control.manager.keyboardModel
                    onAccepted: control.manager.keyboardModel = text
                }
            }

            Maui.SectionItem
            {
                label1.text: i18n("Options")
                label2.text: i18n("Keyboard options, separated by a comma.")
                columns: 1

                TextField
                {
                    Layout.fillWidth: true
                    text: control.manager.keyboardOptions
                    onAccepted: control.manager.keyboardOptions = text
                }
            }

            Maui.SectionItem
            {
                label1.text: i18n("Variant")
                label2.text: i18n("Keyboard variant.")
                columns: 1

                TextField
                {
                    Layout.fillWidth: true
                    text: control.manager.keyboardVariant
                    onAccepted: control.manager.keyboardVariant = text
                }
            }

            Maui.SectionItem
            {
                label1.text: i18n("Rules")
                label2.text: i18n("Keyboard rules, separated by a comma.")
                columns: 1

                TextField
                {
                    Layout.fillWidth: true
                    text: control.manager.keyboardRules
                    onAccepted: control.manager.keyboardRules = text
                }
            }
        }
    }


    Maui.SectionGroup
    {
        title: i18n("Keyboard")

        Maui.SectionItem
        {
            label1.text: i18n("Keymap")
            label2.text: i18n("Keymap options.")

            ToolButton
            {
                checkable: true
                icon.name: "go-next"
                onToggled: control.stackView.push(_keyboardPageComponent)
            }
        }

        Maui.SectionItem
        {
            label1.text: i18n("Hardware")
            label2.text: i18n("Extra options.")
            ToolButton
            {
                checkable: true
                icon.name: "go-next"
                //                onToggled: control.stackView.push(_fontsPageComponent)
            }
        }
    }

}
