// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui

import org.maui.settings.lib 1.0 as ManLib
import org.mauikit.filebrowsing 1.3 as FB

Maui.GridBrowser
{
    id: control

    property var manager

    property string title : i18n("Wallpapers")
    //            implicitHeight: contentHeight
    currentIndex :-1

    itemSize: Math.min(Math.floor(control.width/3), 200)

    model: Maui.BaseModel
    {
        list: FB.FMList
        {
            path: control.manager.wallpaperSourceDir
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
            autoExclusive: true
            checkable: false
            checked: model.thumbnail ===  control.manager.wallpaperSource
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
                control.currentIndex = index
                control.manager.wallpaperSource = model.thumbnail
            }

            onDoubleClicked:
            {
                control.currentIndex = index
                control.manager.wallpaperSource = model.thumbnail
                control.stackView.pop()
            }
        }
    }
}
