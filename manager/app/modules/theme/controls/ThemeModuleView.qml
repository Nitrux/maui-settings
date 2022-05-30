// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui

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
        title: i18n("Color Scheme")

        Maui.SettingTemplate
        {
            label1.text: i18n("Style")
            label2.text: i18n("Pick a style variant.")

            Item
            {
                width: parent.parent.width
                height: _flow.implicitHeight
                GridLayout
                {
                    id: _flow
                    width: parent.width
                    anchors.centerIn: parent
                    columns: 3
                    rows: 2
                    flow:GridLayout.TopToBottom
                    columnSpacing:  Maui.Style.space.medium
                    rowSpacing: columnSpacing
                    ManLib.GraphicButton
                    {
                        checked: control.manager.styleType === 0
                        implicitWidth: 180
                        implicitHeight: 180
                        Layout.fillWidth: true
                        Layout.minimumWidth: 100
                        text: "Light"
                        imageSource: "qrc:/light_mode.png"

                        onClicked: control.manager.styleType = 0
                    }

                    ManLib.GraphicButton
                    {
                        implicitWidth: 180
                        implicitHeight: 180
                        Layout.fillWidth: true
                        Layout.minimumWidth: 100
                        checked: control.manager.styleType === 1
                        imageSource: "qrc:/dark_mode.png"
                        text: "Dark"
                        onClicked: control.manager.styleType = 1
                    }

                    ManLib.GraphicButton
                    {
                        implicitWidth: 180
                        implicitHeight: 180
                        Layout.fillWidth: true
                        Layout.minimumWidth: 100
                        checked: control.manager.styleType === 2
                        text: "Adaptive"
                        imageSource: "qrc:/adaptive_mode.png"

                        onClicked: control.manager.styleType = 2
                    }

                    ManLib.GraphicButton
                    {
                        implicitWidth: 180
                        implicitHeight: 180
                        Layout.fillWidth: true
                        Layout.minimumWidth: 100
                        checked: control.manager.styleType === 3
                        text:"Auto"
                        onClicked: control.manager.styleType = 3
                    }
                }
            }
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Accent Color")
            label2.text: i18n("Custom accent color.")
            Maui.ColorsRow
            {
                width: parent.parent.width
                size: Maui.Style.iconSizes.medium
                colors: ["#26c6da", "#2C3E50", "#1976d2", "#212121", "#8e24aa", "#16A085"]
                currentColor: control.manager.accentColor

                onColorPicked:
                {
                    control.manager.accentColor = color
                }
            }
        }
    }

    Maui.SettingsSection
    {
        title: i18n("Icon Theme")

    }

    Maui.SettingsSection
    {
        title: i18n("Window Control Decorations")
    }
}
