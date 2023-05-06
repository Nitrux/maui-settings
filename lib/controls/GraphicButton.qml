// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import QtGraphicalEffects 1.0
import QtQuick.Dialogs 1.3

import org.mauikit.controls 1.3 as Maui
import QtQuick.Templates 2.15 as T

Button
{
    id: control

    implicitHeight: implicitContentHeight + topPadding + bottomPadding
    implicitWidth: implicitContentWidth + rightPadding + leftPadding

    spacing: Maui.Style.space.medium
    flat: false

    property alias button : _button
    property Component iconComponent : _defaultGraphic

    property int radius: Maui.Style.radiusV
    property color color: "transparent"
    property int fillMode : Image.PreserveAspectFit

    property string imageSource
    property string iconSource

    property bool imageVisible : true
    property int imageWidth :  -1

    Component
    {
        id: _defaultGraphic

        Maui.IconItem
        {
            iconSource: control.iconSource
            imageSource: control.imageSource
            fillMode: control.fillMode
            maskRadius: control.radius
            imageSizeHint: control.imageWidth
            imageWidth: 400
            imageHeight: 200
        }
    }

    contentItem: ColumnLayout
    {
        spacing: control.spacing
        Rectangle
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            implicitWidth: _loader.implicitWidth
            implicitHeight: _loader.implicitHeight

            Loader
            {
                id: _loader
                visible: control.imageVisible
                anchors.fill: parent
                asynchronous: true
                sourceComponent: control.iconComponent
                scale: control.checked ? 0.8 : 1
            }

            Maui.CheckBoxItem
            {
                autoExclusive: control.autoExclusive
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.margins: Maui.Style.space.medium
                checked: control.checked
                checkable: control.checkable
                visible: checked

                height: Maui.Style.iconSizes.medium
                width: height
            }

            radius: control.radius

            readonly property color m_color : Qt.tint(Qt.lighter(Maui.Theme.textColor), Qt.rgba(Maui.Theme.backgroundColor.r, Maui.Theme.backgroundColor.g, Maui.Theme.backgroundColor.b, 0.9))

            color: control.flat ? control.color : (control.checked || control.pressed ? Maui.Theme.highlightColor : ( control.hovered ? Maui.Theme.hoverColor : ( Qt.rgba(m_color.r, m_color.g, m_color.b, 0.4))))


            Behavior on color
            {
                Maui.ColorTransition{}
            }
        }

        Button
        {
            id: _button
            checked: control.checked
            checkable: control.checkable
            Layout.fillWidth: true
            text: control.text
            onClicked: control.clicked()
            autoExclusive: control.autoExclusive
        }
    }

    background: Item
    {

    }
}
