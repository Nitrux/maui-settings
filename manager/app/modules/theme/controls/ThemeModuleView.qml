// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import QtGraphicalEffects 1.0

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
        onClicked:
        {
            control.parent.pop()
        }
    }

    ColumnLayout
    {
        anchors.fill: parent

        Maui.SettingsSection
        {
            title: i18n("Options")


            Maui.SettingTemplate
            {
                label1.text: i18n("Adaptive Colors")
                label2.text: i18n("Use wallpaper to ste the shell color scheme")

                RowLayout
                {
                    width: parent.parent.width

                    Maui.GridBrowserDelegate
                    {
                        implicitWidth: 100
                        implicitHeight: 100
                        label1.text: "Light"
                        onClicked: control.manager.styleType = 0
                    }

                    Maui.GridBrowserDelegate
                    {
                        implicitWidth: 100
                        implicitHeight: 100

                        label1.text: "Dark"
                        onClicked: control.manager.styleType = 1
                    }

                    Maui.GridBrowserDelegate
                    {
                        implicitWidth: 100
                        implicitHeight: 100

                        label1.text: "Adaptive"
                        onClicked: control.manager.styleType = 2
                    }

                    Maui.GridBrowserDelegate
                    {
                        implicitWidth: 100
                        implicitHeight: 100

                        label1.text:"Auto"
                        onClicked: control.manager.styleType = 3
                    }
                }
            }

            Maui.SettingTemplate
            {
                label1.text: i18n("Accent Colors")
                label2.text: i18n("Custom accent color")
                Maui.ColorsRow
                {
                    width: parent.parent.width
                    colors: ["#26c6da", "#2C3E50", "#1976d2", "#212121", "#8e24aa", "#16A085"]
                    currentColor: control.manager.accentColor

                    onColorPicked:
                    {
                        control.manager.accentColor = color
                    }
                }
            }
        }
    }
}
