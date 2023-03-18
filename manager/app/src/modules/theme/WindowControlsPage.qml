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

    title: i18n("Window Controls")

    Maui.SectionItem
    {
        label1.text: i18n("Style")
        label2.text: i18n("Enable CLient Side Decorations for MauiApps.")
        columns : 1

        Flow
        {
            Layout.fillWidth: true
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

    Maui.SectionItem
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
