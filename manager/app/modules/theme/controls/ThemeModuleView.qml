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


    ButtonGroup
    {
        id: _styleGroup
    }

    Maui.SettingsSection
    {
        title: i18n("Style")
        description: i18n("Pick a style variant.")

        Maui.SettingTemplate
        {
            label1.text: i18n("Light")
            label2.text: i18n("Light colors tinted by the accent color.")
            template.imageSource: "qrc:/light_mode.png"

            RadioButton
            {
                //                autoExclusive: true
                checked: control.manager.styleType === 0
                ButtonGroup.group: _styleGroup
                onToggled: control.manager.styleType = 0
            }
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Dark")
            label2.text: i18n("Dark colors tinted by the accent color.")
            template.imageSource: "qrc:/dark_mode.png"

            RadioButton
            {
                //                autoExclusive: true
                checked: control.manager.styleType === 1
                ButtonGroup.group: _styleGroup
                onToggled: control.manager.styleType = 1
            }
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Adaptive")
            label2.text: i18n("Light colors tinted by the accent color.")
            template.imageSource: "qrc:/adaptive_mode.png"

            RadioButton
            {
                autoExclusive: true
                checked: control.manager.styleType === 2
                ButtonGroup.group: _styleGroup
                onToggled: control.manager.styleType = 2


            }
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Auto")
            label2.text: i18n("Light colors tinted by the accent color.")
            template.imageSource: "qrc:/auto_mode.png"

            RadioButton
            {
                autoExclusive: true
                checked: control.manager.styleType === 3
                ButtonGroup.group: _styleGroup
                onToggled: control.manager.styleType = 3

            }
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Border Radius")
            label2.text: i18n("Custom border radius.")

            SpinBox
            {
                from: 0
                to: 20
                value: control.manager.borderRadius
                stepSize: 1
                onValueModified: control.manager.borderRadius = value
            }
        }
    }

    Maui.SettingsSection
    {
        title: i18n("Colors")

        Maui.SettingTemplate
        {
            label1.text: i18n("Accent Color")
            label2.text: i18n("Custom accent color.")
            Maui.ColorsRow
            {
                width: parent.parent.width
                size: Maui.Style.iconSizes.medium*1.2
                colors: ["#26c6da", "#2C3E50", "#1976d2", "#212121", "#8e24aa", "#16A085", "#BBCDE5", "#E6BCC3", "#FEF9A7", "#00FFAB", "#2A2550", "#827397", "#FF6363", "#F900BF", "#FFF56D"]
                currentColor: control.manager.accentColor

                onColorPicked:
                {
                    control.manager.accentColor = color
                }
            }
        }

        Maui.Chip
        {
            visible: control.manager.styleType === 2
            text: i18n("Accent color is not used with the Adaptive Style.")
            color: Maui.Theme.neutralBackgroundColor
            iconSource: "dialog-warning"
        }
    }

    Maui.SettingsSection
    {
        title: i18n("Icon Theme")

    }

    Maui.SettingsSection
    {
        title: i18n("Window Control Decorations")
        visible: !Maui.Handy.isMobile

        Maui.SettingTemplate
        {
            label1.text: i18n("Style")
            label2.text: i18n("Enable CLient Side Decorations for MauiApps.")
            Flow
            {
                width: parent.parent.width
                spacing: Maui.Style.space.medium

                Repeater
                {
                    model: control.manager.windowDecorationsModel

                    delegate: Item
                    {
                        width: _layoutWD.implicitWidth
                        height: _layoutWD.implicitHeight

                        Column
                        {
                            id: _layoutWD
                            spacing: Maui.Style.space.medium

                            Control
                            {
                                Maui.Theme.colorSet: Maui.Theme.Header
                                Maui.Theme.inherit: false
                                padding: Maui.Style.space.medium
                                background: Rectangle
                                {
                                    radius: Maui.Style.radiusV
                                    color: Maui.Theme.backgroundColor
                                }

                                contentItem: Loader
                                {
                                    property bool isActiveWindow: true
                                    property bool maximized: false
                                    property var buttonsModel : Maui.App.controls.rightWindowControls
                                    asynchronous: true
                                    source: model.source
                                }
                            }

                            Button
                            {
                                width: parent.width
                                checked: model.name === Maui.App.controls.styleName
                                text: model.name
                                onClicked: control.manager.windowControlsTheme = model.name
                            }
                        }

                        RadioButton
                        {
                            autoExclusive: true
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.margins: Maui.Style.space.medium

                            visible: model.name === Maui.App.controls.styleName
                            checked: visible
                        }
                    }
                }
            }
        }

        Maui.SettingTemplate
        {
            label1.text: i18n("Use CSD")
            label2.text: i18n("Enable CLient Side Decorations for MauiApps.")

            Switch
            {
                checked: control.manager.enableCSD
                onToggled: control.manager.enableCSD = !control.manager.enableCSD
            }
        }
    }

    Maui.SettingsSection
    {
        title: i18n("Fonts")

    }
}
