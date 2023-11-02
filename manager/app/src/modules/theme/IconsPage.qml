// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

import org.maui.settings.lib as ManLib

Loader
{
    id: control
    property string title: i18n("Icon Theme")
    property var manager
    property Component searchFieldComponent  :  Maui.SearchField
    {
        placeholderText: i18n ("Filter")
        onAccepted: control.manager.iconsModel.filter = text
        onCleared: control.manager.iconsModel.filter = undefined
    }

    asynchronous: true

    sourceComponent: Maui.GridBrowser
    {
        itemSize: 144
        itemHeight: 120

        model: control.manager.iconsModel
        delegate: Item
        {
            width: GridView.view.cellWidth
            height: GridView.view.itemHeight

            Maui.GridBrowserDelegate
            {
                id: _iconsDelegate
                property var iconsModel : model.icons
                checked:  Maui.Style.currentIconTheme === model.themeName
                anchors.fill: parent
                anchors.margins: Maui.Style.space.medium
                flat: false

                template.iconComponent: Control
                {
                    implicitHeight: Math.max(_layout.implicitHeight + topPadding + bottomPadding, 64)
                    padding: Maui.Style.space.small

                    background: Rectangle
                    {
                        color: Maui.Theme.alternateBackgroundColor//first color is the window background color
                        radius: Maui.Style.radiusV
                    }

                    contentItem: GridLayout
                    {
                        id: _layout
                        rows: 2
                        columns: 3
                        columnSpacing: Maui.Style.space.small
                        rowSpacing: Maui.Style.space.small

                        Repeater
                        {
                            model: _iconsDelegate.iconsModel
                            delegate: Image
                            {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                sourceSize.height: 48
                                sourceSize.width: 48
                                fillMode: Image.PreserveAspectFit

                                asynchronous: true
                                source: modelData


                            }
                        }
                    }
                }

                onClicked: control.manager.iconTheme = model.themeName

                label1.text: model.display
            }
        }
    }
}

