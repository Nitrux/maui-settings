// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui

import org.maui.settings.lib 1.0 as ManLib

Maui.SettingsPage
{
    id: control

    property var manager

    title: i18n("Units")
    Maui.SectionItem
    {
        label1.text: i18n("Effects")
        label2.text: i18n("Enable effects.")

        Switch
        {
            checked: control.manager.enableEffects
            onToggled: control.manager.enableEffects = !control.manager.enableEffects
        }
    }

    Maui.SectionItem
    {
        label1.text: i18n("Border Radius")
        label2.text: i18n("Custom border radius.")
        columns : 3

        SpinBox
        {
            from: 0
            to: 20
            value: control.manager.borderRadius
            stepSize: 1
            onValueModified: control.manager.borderRadius = value
        }

        Button
        {
            text: i18n("Reset")
            onClicked: control.manager.borderRadius = undefined
        }
    }


    Maui.SectionItem
    {
        label1.text: i18n("Elements Size")
        label2.text: i18n("Size of the UI elements.")
        columns : 3

        Maui.ToolActions
        {
            autoExclusive: true
            expanded: true

            Action
            {
                text: i18n("S")
                checked: control.manager.paddingSize === 4
                onTriggered: control.manager.paddingSize = 4
            }

            Action
            {
                text: i18n("M")
                checked: control.manager.paddingSize === 6
                onTriggered: control.manager.paddingSize = 6
            }

            Action
            {
                text: i18n("L")
                checked: control.manager.paddingSize === 8
                onTriggered: control.manager.paddingSize = 8
            }
        }

        Button
        {
            text: i18n("Reset")
            onClicked: control.manager.paddingSize = undefined
        }
    }

    Maui.SectionItem
    {
        label1.text: i18n("Content Margins")
        label2.text: i18n("Margins of the views.")
        columns : 3

        Maui.ToolActions
        {
            autoExclusive: true
            expanded: true

            Action
            {
                text: i18n("S")
                checked: control.manager.marginSize === 4
                onTriggered: control.manager.marginSize = 4
            }

            Action
            {
                text: i18n("M")
                checked: control.manager.marginSize === 6
                onTriggered: control.manager.marginSize = 6
            }

            Action
            {
                text: i18n("L")
                checked: control.manager.marginSize === 8
                onTriggered: control.manager.marginSize = 8
            }
        }

        Button
        {
            text: i18n("Reset")
            onClicked: control.manager.marginSize = undefined
        }
    }

    Maui.SectionItem
    {
        label1.text: i18n("Content Spacing")
        label2.text: i18n("Spacing between elements, such as list, tab buttons.")
        columns : 3

        Maui.ToolActions
        {
            autoExclusive: true
            expanded: true

            Action
            {
                text: i18n("S")
                checked: control.manager.spacingSize === 4
                onTriggered: control.manager.spacingSize = 4
            }

            Action
            {
                text: i18n("M")
                checked: control.manager.spacingSize === 6
                onTriggered: control.manager.spacingSize = 6
            }

            Action
            {
                text: i18n("L")
                checked: control.manager.spacingSize === 8
                onTriggered: control.manager.spacingSize = 8
            }
        }

        Button
        {
            text: i18n("Reset")
            onClicked: control.manager.spacingSize = undefined
        }
    }
}

