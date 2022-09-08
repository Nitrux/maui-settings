// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15

import org.mauikit.controls 1.3 as Maui
import org.maui.settings.lib 1.0 as ManLib

Maui.Page
{
    id: control
    default property alias content: _template.content
    property alias stackView: _stackView
    showCSDControls: true
//    headBar.background: null
    headBar.leftContent:[ToolButton
        {
            icon.name: "sidebar-collapse"

            onClicked: toggleSideBar()
        },
        ToolButton
        {
            icon.name: "go-previous"

            onClicked:
            {
                if(_stackView.depth > 1)
                {
                    _stackView.pop()
                }else
                {
                    control.StackView.view.pop()
                }
            }
        }
    ]

    Maui.StackView
    {
        id: _stackView
        anchors.fill: parent

        initialItem: ManLib.SettingsLayout
        {
            id: _template
        }
    }
}
