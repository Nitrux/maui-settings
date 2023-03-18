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

    property string imageSource
    property color backgroundColor
    property color foregroundColor
    property color highlightColor

    property int previewHeight
    property int previewWidth

    iconComponent: Control
    {
        implicitHeight: control.previewHeight
        implicitWidth: control.previewWidth

        padding: Maui.Style.space.medium

        background: Rectangle
        {
            color: Maui.Theme.backgroundColor
            radius: Maui.Style.radiusV
            Maui.IconItem
            {
                anchors.fill: parent

                imageSource:  control.imageSource
                fillMode: Image.PreserveAspectCrop
                maskRadius: 10
            }
        }

        contentItem: ColumnLayout
        {
            Button
            {
                Layout.fillWidth: true
                icon.name: "love"
                icon.color: control.foregroundColor

                background: Rectangle
                {
                    radius: 10
                    color: control.backgroundColor
                    opacity: 0.5
                }
            }

            Item
            {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Rectangle
            {
                color: control.highlightColor

                Layout.fillWidth: true
                Layout.maximumHeight: 24
                Layout.preferredHeight: 24

                radius: 10
            }

            Rectangle
            {
                color: control.backgroundColor

                Layout.fillWidth: true
                Layout.maximumHeight: 64
                Layout.preferredHeight: 48
                radius: 10
            }
        }
    }
}
