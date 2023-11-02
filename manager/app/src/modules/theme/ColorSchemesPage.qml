// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

import org.mauikit.controls as Maui

import org.maui.settings.lib as ManLib

Maui.GridBrowser
{
    id: control

    property var manager
    readonly property string title: "Color Schemes"

    property Component searchFieldComponent: Maui.SearchField
    {
        placeholderText: i18n ("Filter")
        onAccepted: control.manager.colorSchemeModel.filter = text
        onCleared: control.manager.colorSchemeModel.filter = undefined
    }

    model: control.manager.colorSchemeModel

    itemSize: 120
    itemHeight: 140

    delegate:  Item
    {

        width: GridView.view.cellWidth
        height: GridView.view.itemHeight

        Maui.GridBrowserDelegate
        {
            id: _delegate
            anchors.fill: parent
            anchors.margins: Maui.Style.space.medium
            checked: model.file === control.manager.customColorScheme
            onClicked:  control.manager.customColorScheme = model.file

            readonly property var colors : model.preview

            template.iconComponent: Control
            {
                implicitHeight: Math.max(_layout.implicitHeight + topPadding + bottomPadding, 64)
                padding: Maui.Style.space.small

                background: Rectangle
                {
                    color: _delegate.colors[0] //first color is the window background color
                    radius: Maui.Style.radiusV
                }

                contentItem: Column
                {
                    id:_layout
                    spacing: 2
                    clip: true
                    Label
                    {
                        width: parent.width
                        text: i18n ("Hello")
                        color: _delegate.colors[1]
                    }

                    Rectangle
                    {
                        radius: 2
                        height: 16
                        width: parent.width
                        color: _delegate.colors[2]
                    }

                    Rectangle
                    {
                        radius: 2
                        height: 6
                        width: parent.width
                        color: _delegate.colors[4]
                    }

                    Rectangle
                    {
                        radius: 2
                        height: 26
                        width: parent.width
                        color: _delegate.colors[3]
                    }

                }
            }

            label1.text: model.name
        }
    }
}
