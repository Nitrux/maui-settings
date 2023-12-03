// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.maui.settings.lib as ManLib

Maui.Page
{
    id: control
    default property alias content: _template.content
    property alias stackView: _stackView

    property var module
    readonly property string moduleId : module ? module.id : ""

    title: _stackView.currentItem.title ? _stackView.currentItem.title : ""
    Maui.Controls.showCSD: true
    showTitle: !_searchFieldLoader.visible

    headBar.leftContent:[ToolButton
        {
            icon.name: sideBar.visible ? "sidebar-collapse" : "sidebar-expand"
            checked: sideBar.visible
            visible: sideBar.collapsed
            onClicked: toggleSideBar()
        },

        ToolButton
        {
            icon.name: "go-previous"
            onClicked: _stackView.pop()
            visible: _stackView.depth > 1
            text: _stackView.depth > 1 ? (_stackView.get(_stackView.currentItem.StackView.index-1, StackView.DontLoad).title) : ""
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
