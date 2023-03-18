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

import org.maui.settings 1.0 as Man
import org.maui.settings.lib 1.0 as ManLib

ManLib.SettingsPage
{
    id: control

    property var manager : control.module.manager


    Maui.SectionGroup
    {
        title: i18n("Wallpaper Image")
        description: i18n("Pick a new image")
        RowLayout
        {
            //                        Layout.margins: Maui.Style.space.medium
            Layout.fillWidth: true
            Layout.fillHeight: false
            Layout.preferredHeight: 180
            Layout.preferredWidth: 400
            Layout.maximumWidth: 800
            Layout.alignment: Qt.AlignCenter
            spacing: Maui.Style.space.big


            ManLib.GraphicButton
            {
                id: _imgDesktop
                flat: true
                Layout.fillHeight: true
                Layout.fillWidth: true
                imageVisible: control.manager.showWallpaper
                radius: fillMode  === Image.PreserveAspectFit ? 0 : 8

                color: control.manager.solidColor
                fillMode: control.manager.fitWallpaper ? Image.PreserveAspectFit :Image.PreserveAspectCrop
                imageSource: control.manager.wallpaperSource
                text: i18n("Workspace")

                onClicked: control.stackView.push(_wallpapersGridViewComponent)

            }

            ManLib.GraphicButton
            {
                radius: 8
                color: "#333"
                Layout.fillHeight: true
                Layout.fillWidth: true
                fillMode: control.manager.fitWallpaper ? Image.PreserveAspectFit :Image.PreserveAspectCrop
                imageSource: control.manager.wallpaperSource
                text: i18n("Lock Screen")
                onClicked: control.stackView.push(_wallpapersGridViewComponent)


            }
        }
    }

    Maui.SectionGroup
    {
        title: i18n("Options")
        description: i18n("Configure look and feel")

        Maui.SectionItem
        {
            label1.text: i18n("Dim Wallpaper")
            label2.text: i18n("On dark mode the wallpaper will be dimmed")

            Switch
            {
                checked: control.manager.dimWallpaper
                onToggled: control.manager.dimWallpaper = !control.manager.dimWallpaper
            }
        }

        Maui.SectionItem
        {
            label1.text: i18n("Fit Wallpaper")
            label2.text: i18n("Fill or fit wallpaper")

            Switch
            {
                checked: control.manager.fitWallpaper
                onToggled: control.manager.fitWallpaper = !control.manager.fitWallpaper
            }
        }

        Maui.SectionItem
        {
            label1.text: i18n("Show Wallpaper")
            label2.text: i18n("Show image or solid color")

            Switch
            {
                checked: control.manager.showWallpaper
                onToggled: control.manager.showWallpaper = !control.manager.showWallpaper
            }
        }
    }

    Maui.SectionGroup
    {
        title: i18n("Customize")
        Maui.SectionItem
        {
            enabled: control.manager.showWallpaper

            label1.text: i18n("Solid Color")
            label2.text: i18n("Pick a solid color")

            Button
            {
                background: Rectangle
                {
                    radius: Maui.Style.radiusV
                    color: control.manager.solidColor
                }

                onClicked: colorDialog.open()
            }

            ColorDialog
            {
                id: colorDialog
                title: "Please choose a color"
                onAccepted:
                {
                    control.manager.solidColor = colorDialog.color
                    console.log("You chose: " + colorDialog.color)
                }
                onRejected: {
                    console.log("Canceled")
                    colorDialog.close()
                }
            }
        }

        Maui.SectionItem
        {
            label1.text: i18n("Sources")
            label2.text: i18n("Wallpaper source directory")
            columns: 1
            RowLayout
            {
                width: parent.parent.width
                spacing: Maui.Style.space.medium

                TextField
                {
                    id: _sourceDirField
                    Layout.fillWidth: true
                    text: control.manager.wallpaperSourceDir

                    actions: Action
                    {
                        icon.name: "checkbox"
                        onTriggered: control.manager.wallpaperSourceDir = _sourceDirField.text
                    }
                }

                Button
                {
                    text: i18n("Reset")
                    onClicked: control.manager.wallpaperSourceDir = undefined
                }
            }
        }
    }

    Component
    {
        id: _wallpapersGridViewComponent

        WallpapersPage
        {
            manager: control.manager
        }
    }
}
