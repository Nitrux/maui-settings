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
                    width: Math.min(implicitWidth, parent.width)
                    anchors.centerIn: parent
                    columns: 2
                    rows: 2
                    flow:GridLayout.TopToBottom
                    columnSpacing:  Maui.Style.space.medium
                    rowSpacing: columnSpacing

                    property int itemSize : 160
                    ManLib.GraphicButton
                    {
                        checked: control.manager.styleType === 0
                        implicitWidth: _flow.itemSize
                        implicitHeight: _flow.itemSize
                        Layout.fillWidth: true
                        text: "Light"
                        imageSource: "qrc:/light_mode.png"
                        imageWidth: 80
                        onClicked: control.manager.styleType = 0
                    }

                    ManLib.GraphicButton
                    {
                        implicitWidth: _flow.itemSize
                        implicitHeight: _flow.itemSize
                        Layout.fillWidth: true

                        checked: control.manager.styleType === 1
                        imageSource: "qrc:/dark_mode.png"
                        text: "Dark"
                        imageWidth: 80

                        onClicked: control.manager.styleType = 1
                    }

                    ManLib.GraphicButton
                    {
                        implicitWidth: _flow.itemSize
                        implicitHeight: _flow.itemSize
                        Layout.fillWidth: true

                        checked: control.manager.styleType === 2
                        text: "Adaptive"
                        imageSource: "qrc:/adaptive_mode.png"
                        imageWidth: 80

                        onClicked: control.manager.styleType = 2
                    }

                    ManLib.GraphicButton
                    {
                        implicitWidth: _flow.itemSize
                        implicitHeight: _flow.itemSize
                        Layout.fillWidth: true

                        checked: control.manager.styleType === 3
                        text:"Auto"
                        imageWidth: 80

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
                size: Maui.Style.iconSizes.medium*1.5
                colors: ["#26c6da", "#2C3E50", "#1976d2", "#212121", "#8e24aa", "#16A085", "#BBCDE5", "#E6BCC3", "#FEF9A7", "#00FFAB", "#2A2550", "#827397", "#FF6363", "#F900BF", "#FFF56D"]
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
                                    radius: model.radius
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

                        Maui.CheckBoxItem
                        {
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
}
