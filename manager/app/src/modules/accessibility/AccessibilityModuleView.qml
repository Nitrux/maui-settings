// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Qt5Compat.GraphicalEffects

import org.mauikit.controls as Maui
import org.maui.settings.lib as ManLib

ManLib.SettingsPage
{
    id: control

    property var manager : control.module.manager

    Maui.SectionGroup
    {
        title: i18n("Navigation")
        description: i18n("User experience interactions")

        Maui.SectionItem
        {
            label1.text: i18n("Single Click")
            label2.text: i18n("Open items with a single click, otherwise with a double click.")

            Switch
            {
                checked: control.manager.singleClick
                onToggled: control.manager.singleClick = !control.manager.singleClick
            }
        }

        Maui.SectionItem
        {
            label1.text: i18n("Scroll Bars Policy")
            label2.text: i18n("Scrolling bars visibility and behaviour.")

            columns:1

            ColumnLayout
            {
                Layout.fillWidth: true

                Maui.SectionItem
                {
                    label1.text: i18n("Always On")

                    onClicked: control.manager.scrollBarPolicy = 0

                    CheckBox
                    {
                        autoExclusive: true
                        checked: control.manager.scrollBarPolicy === 0
                    }
                }

                Maui.SectionItem
                {
                    label1.text: i18n("As Needed")

                    onClicked: control.manager.scrollBarPolicy = 1

                    CheckBox
                    {
                        autoExclusive: true
                        checked: control.manager.scrollBarPolicy === 1
                    }
                }

                Maui.SectionItem
                {
                    label1.text: i18n("Always Off")

                    onClicked: control.manager.scrollBarPolicy = 2

                    CheckBox
                    {
                        autoExclusive: true
                        checked: control.manager.scrollBarPolicy === 2
                    }
                }

                Maui.SectionItem
                {
                    label1.text: i18n("Auto Hide")

                    onClicked: control.manager.scrollBarPolicy = 3

                    CheckBox
                    {
                        autoExclusive: true
                        checked: control.manager.scrollBarPolicy === 3
                    }
                }

            }
        }
    }



}
