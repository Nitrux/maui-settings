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

ManLib.GraphicButton
{
    id: control

    property string imageSource : manager.wallpaper

    property color backgroundColor
    property color foregroundColor
    property color highlightColor

    property int previewHeight : 200
    property int previewWidth : 400

    autoExclusive: true

    Layout.maximumWidth: 250
    Layout.minimumWidth: 84
    Layout.fillWidth: true
    Layout.fillHeight: true

    iconComponent: Control
    {
        implicitHeight: control.previewHeight
        implicitWidth: control.previewWidth

        padding: Maui.Style.defaultPadding

        background: Rectangle
        {
            color: control.backgroundColor
            radius: control.radius

            Maui.IconItem
            {
                anchors.fill: parent

                imageSource:  control.imageSource
                fillMode: Image.PreserveAspectCrop
                maskRadius: control.radius
                imageWidth: 400
                imageHeight: 200
                opacity: 0.7
            }

            layer.enabled: Maui.Style.enableEffects
            layer.effect: DropShadow
            {
                horizontalOffset: 0
                verticalOffset: 0
                radius: 8
                samples: 16
                color: "#80000000"
                transparentBorder: true
            }
        }

        contentItem: ColumnLayout
        {
            clip: true

            Rectangle
            {
                Layout.fillWidth: true
                radius: control.radius
                implicitHeight: 24
                color: control.backgroundColor
            }

            Item
            {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Rectangle
            {
                color: control.backgroundColor

                Layout.fillWidth: true
                Layout.maximumHeight: 100
                Layout.preferredHeight: 48
                Layout.fillHeight: true
                radius: control.radius

                Column
                {
                    spacing: Maui.Style.space.medium
                    anchors
                    {
                        fill: parent
                        margins: Maui.Style.defaultPadding
                    }

                    Rectangle
                    {
                        color: control.highlightColor
                        width: parent.width

                        height: 24
                        radius: control.radius
                    }

                    Rectangle
                    {
                        color: control.foregroundColor
                        width: parent.width
                        height: 24
                        radius: control.radius
                    }
                }
            }
        }
    }
}
