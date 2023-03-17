// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import org.mauikit.controls 1.3 as Maui
import org.maui.settings.lib 1.0 as ManLib

Maui.Page
{
    id: control
    default property alias content: _template.content
    property alias stackView: _stackView

    property var module
    readonly property string moduleId : module ? module.id : ""

    title: _stackView.currentItem.title ? _stackView.currentItem.title : ""
    showCSDControls: true
    showTitle: !_searchFieldLoader.visible

    headBar.leftContent:[ToolButton
        {
            icon.name: "sidebar-collapse"

            onClicked: toggleSideBar()
        },

        ToolButton
        {
            icon.name: "go-previous"
            onClicked: _stackView.pop()
            visible:  _stackView.depth > 1
        }
    ]

    headBar.middleContent: Loader
    {
        id: _searchFieldLoader
        visible: item
        sourceComponent: _stackView.currentItem && _stackView.currentItem.hasOwnProperty("searchFieldComponent") ? _stackView.currentItem.searchFieldComponent : null
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        Layout.maximumWidth: 500
    }


    StackView
    {
        id: _stackView
        anchors.fill: parent

        initialItem: Maui.SettingsPage
        {
            id: _template
            spacing: Maui.Style.space.huge
            title: module ? module.name : ""

        }
    }
}
