// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui

import org.maui.settings.lib 1.0 as ManLib

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

    iconComponent: Control
    {
        implicitHeight: control.previewHeight
        implicitWidth: control.previewWidth

        padding: Maui.Style.space.medium

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
        }

        contentItem: ColumnLayout
        {
            clip: true

            Rectangle
            {
                Layout.fillWidth: true
                radius: 10
                implicitHeight: 24
                color: control.backgroundColor
                opacity: 0.5
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
                radius: 10


                Column
                {
                    spacing: Maui.Style.space.medium
                    anchors
                    {
                        fill: parent
                        margins: Maui.Style.space.medium
                    }

                    Rectangle
                    {
                        color: control.highlightColor
                        width: parent.width

                        height: 24
                        radius: 10
                    }

                    Rectangle
                    {
                        color: control.foregroundColor
                        width: parent.width
                        height: 24
                        radius: 10
                    }
                }


            }
        }
    }
}
