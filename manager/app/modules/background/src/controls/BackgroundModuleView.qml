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
import org.mauikit.filebrowsing 1.3 as FB
import org.kde.kirigami 2.14 as Kirigami
import org.maui.manager 1.0 as Man

Maui.Page
{
    id: control

    property var module
    property var manager : control.module.manager
    title: module.name

    headBar.leftContent: ToolButton
    {
        icon.name: "go-previous"
        //        visible: control.depth >= 1
        onClicked:
        {
            if(_stackView.depth > 1)
            {
                _stackView.pop()
            }else
            {
                control.parent.pop()
            }

        }
    }

    Maui.StackView
    {
        id: _stackView
        anchors.fill: parent

        initialItem: ScrollView
        {
            Flickable
            {
                contentWidth: availableWidth
                contentHeight: _initialLayout.implicitHeight

                ColumnLayout
                {
                    id: _initialLayout
                    width: parent.width
                    spacing: Maui.Style.space.medium

                    Maui.SettingsSection
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
                            Layout.alignment: Qt.AlignCenter
                            spacing: Maui.Style.space.big

                            ColumnLayout
                            {
                                Layout.fillHeight: true
                                Layout.fillWidth: true

                                Rectangle
                                {
                                    id: _imgDesktop

                                    Layout.fillHeight: true
                                    Layout.fillWidth: true

                                    radius: 8
                                    color: "#333"

                                    Image
                                    {
                                        anchors.fill: parent
                                        fillMode: control.manager.fitWallpaper ? Image.PreserveAspectFit :Image.PreserveAspectCrop
                                        source: control.manager.wallpaperSource
                                    }

                                    Rectangle
                                    {
                                        anchors.top: parent.top
                                        color: Kirigami.Theme.backgroundColor
                                        opacity: 0.7
                                        height: 10
                                        radius: 4
                                        width: parent.width* 0.95
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.margins: 6

                                    }

                                    Rectangle
                                    {
                                        anchors.bottom: parent.bottom
                                        color: Kirigami.Theme.backgroundColor
                                        opacity: 0.7
                                        height: 30
                                        radius: 10
                                        width: parent.width* 0.6
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.margins: 6
                                    }


                                    layer.enabled: true
                                    layer.effect: OpacityMask
                                    {
                                        maskSource: Rectangle
                                        {
                                            width: _imgDesktop.width
                                            height: _imgDesktop.height
                                            radius: 8
                                        }
                                    }
                                }


                                Button
                                {
                                    Layout.alignment: Qt.AlignCenter
                                    Layout.fillWidth: true
                                    text: i18n("Workspace")
                                    onClicked: _stackView.push(_wallpapersGridViewComponent)
                                }

                            }

                            ColumnLayout
                            {

                                Layout.fillHeight: true
                                Layout.fillWidth: true

                                Rectangle
                                {
                                    id: _imgLockScreen

                                    Layout.fillHeight: true
                                    Layout.fillWidth: true
                                    radius: 8
                                    color: "#333"

                                    Image
                                    {
                                        anchors.fill: parent
                                        fillMode: control.manager.fitWallpaper ? Image.PreserveAspectFit :Image.PreserveAspectCrop
                                        source: control.manager.wallpaperSource
                                    }


                                    Column
                                    {

                                        anchors.centerIn: parent
                                        spacing: Maui.Style.space.medium

                                        Rectangle
                                        {
                                            color: Kirigami.Theme.backgroundColor
                                            opacity: 0.7
                                            implicitHeight: 80
                                            implicitWidth: height
                                            radius: 20
                                        }

                                        Rectangle
                                        {
                                            color: Kirigami.Theme.backgroundColor
                                            opacity: 0.7
                                            implicitHeight: 20
                                            implicitWidth: 120
                                            radius: 4
                                        }
                                    }

                                    layer.enabled: true
                                    layer.effect: OpacityMask
                                    {
                                        maskSource: Rectangle
                                        {
                                            width: _imgLockScreen.width
                                            height: _imgLockScreen.height
                                            radius: 8
                                        }
                                    }
                                }

                                Button
                                {
                                    Layout.alignment: Qt.AlignCenter
                                    Layout.fillWidth: true
                                    text: i18n("Lock Screen")
                                    onClicked: _stackView.push(_wallpapersGridViewComponent)
                                }
                            }
                        }
                    }

                    Maui.SettingsSection
                    {
                        title: i18n("Options")
                        description: i18n("Configure look and feel")

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Dim Wallpaper")
                            label2.text: i18n("On dark mode the wallpaper will be dimmed")

                            Switch
                            {
                                checked: control.manager.dimWallpaper
                                onToggled: control.manager.dimWallpaper = !control.manager.dimWallpaper
                            }
                        }

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Fit Wallpaper")
                            label2.text: i18n("Fill or fit wallpaper")

                            Switch
                            {
                                checked: control.manager.fitWallpaper
                                onToggled: control.manager.fitWallpaper = !control.manager.fitWallpaper
                            }
                        }

                        Maui.SettingTemplate
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

                    Maui.SettingsSection
                    {
                        title: i18n("Customize")
                        Maui.SettingTemplate
                        {
                            enabled: control.manager.showWallpaper

                            label1.text: i18n("Solid Color")
                            label2.text: i18n("Pick a solid color")

                            Button
                            {
                                height: Maui.Style.rowHeight
                                width: height

                                background:Rectangle
                                {
                                    radius: Maui.Style.radiusV
                                color: control.manager.solidColor
                                border.color: Kirigami.Theme.textColor
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

                        Maui.SettingTemplate
                        {
                            label1.text: i18n("Sources")
                            label2.text: i18n("Wallpaper source directory")

                            Maui.TextField
                            {
                                width: parent.parent.width
//                                height: implicitHeight
//                                iconSource: "folder"
                                text: control.manager.wallpaperSourceDir
                            }
                        }
                    }


                }
            }
        }
        Component
        {
            id: _wallpapersGridViewComponent

            Maui.GridView
            {
                id: _wallpapersGridView

                //            implicitHeight: contentHeight
                currentIndex :-1

                itemSize: Math.floor(width/3)

                model: Maui.BaseModel
                {
                    list: FB.FMList
                    {
                        path: control.module.manager.wallpaperSourceDir
                        filters: FB.FM.nameFilters(FB.FMList.IMAGE)
                        //                        limit: 40
                    }
                }

                delegate: Item
                {
                    height: GridView.view.cellHeight
                    width: GridView.view.cellWidth
                    property string wallpaper : model.url
                    Maui.GridBrowserDelegate
                    {
                        checkable: false
                        checked: model.thumbnail ===  control.module.manager.wallpaperSource
                        isCurrentItem: parent.GridView.isCurrentItem
                        anchors.fill: parent
                        anchors.margins: Maui.Style.space.tiny
                        iconSource: model.icon
                        imageSource: model.thumbnail
                        //                    iconSizeHint: Maui.Style.iconSizes.medium
                        label1.text: model.label
                        //                    template.labelSizeHint: 32
                        labelsVisible:false
                        tooltipText: model.label

                        onClicked:
                        {
                            _wallpapersGridView.currentIndex = index
                            control.module.manager.wallpaperSource = model.thumbnail
                        }

                        onDoubleClicked:
                        {
                            _wallpapersGridView.currentIndex = index
                            control.module.manager.wallpaperSource = model.thumbnail
                            _stackView.pop()
                        }
                    }
                }
            }
        }
    }
}
