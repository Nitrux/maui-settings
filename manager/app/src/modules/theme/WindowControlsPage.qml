// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

import org.maui.settings.lib as ManLib

Maui.SettingsPage
{
    id: control

    property var manager

    title: i18n("Window Controls")

    Maui.SectionItem
    {
        label1.text: i18n("Style")
        label2.text: i18n("Enable CLient Side Decorations for MauiApps.")

        Flow
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.big

            Repeater
            {
                model: control.manager.windowDecorationsModel

                delegate: Item
                {
                    width: _layoutWD.implicitWidth
                    height: Math.max(_layoutWD.implicitHeight, 80)

                    ColumnLayout
                    {
                        id: _layoutWD
                        anchors.fill: parent
                        spacing: Maui.Style.space.medium

                        Control
                        {
                            Layout.alignment: Qt.AlignCenter
                            Maui.Theme.colorSet: Maui.Theme.Header
                            Maui.Theme.inherit: false
                            padding: Maui.Style.space.small
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
                             Layout.alignment: Qt.AlignBottom
                            Layout.fillWidth: true
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

    Maui.FlexSectionItem
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
