// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

import org.maui.settings.lib as ManLib

ManLib.SettingsPage
{
    id: control

    property var manager : control.module.manager


    Component
    {
        id: _iconsPageComponent
        IconsPage
        {
            manager: control.manager
        }
    }

    Component
    {
        id: _unitsPageComponent
        UnitsPage
        {
            manager: control.manager
        }
    }

    Component
    {
        id: _windowControlsPageComponent

        WindowControlsPage
        {
            manager: control.manager
        }
    }

    Component
    {
        id: _colorSchemesPageComponent

        ColorSchemesPage
        {
            manager: control.manager
        }
    }

    Component
    {
        id: _fontsPageComponent
        FontsPage
        {
            manager: control.manager
        }
    }

    ButtonGroup
    {
        id: _styleGroup
    }

    Maui.SectionGroup
    {
        id: _styleSection
        title: i18n("Style")
        description: i18n("Pick a style variant.")

        GridLayout
        {
            id: _styleLayout
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(control.height * 0.6, 400)

            columns: 3

            columnSpacing: Maui.Style.space.big
            rowSpacing: Maui.Style.space.big

            StyleTypeButton
            {
                ButtonGroup.group: _styleGroup

                text: i18n("Light")

                foregroundColor: "#333"
                backgroundColor: "#fafafa"
                highlightColor: control.manager.accentColor

                checked: control.manager.styleType === 0
                onClicked: control.manager.styleType = 0
            }

            StyleTypeButton
            {
                text: i18n("Dark")

                checked: control.manager.styleType === 1
                onClicked: control.manager.styleType = 1

                ButtonGroup.group: _styleGroup

                foregroundColor: "#fafafa"
                backgroundColor: "#333"
                highlightColor: control.manager.accentColor
            }

            StyleTypeButton
            {
                text: i18n("Adaptive")

                checked: control.manager.styleType === 2
                onClicked: control.manager.styleType = 2

                ButtonGroup.group: _styleGroup

                foregroundColor: _imgColors.foreground
                backgroundColor: _imgColors.background
                highlightColor: _imgColors.average

                Maui.ImageColors
                {
                    id: _imgColors
                    source: control.manager.wallpaper.replace("file://", "")
                    onPaletteChanged:
                    {
                        console.log(_imgColors.average)
                    }
                }
            }

            StyleTypeButton
            {
                text: i18n("Custom")
                checked: control.manager.styleType === 3
                onClicked: control.manager.styleType = 3
                ButtonGroup.group: _styleGroup

                foregroundColor: "#333"
                backgroundColor: "#f4f2f8"
                highlightColor: "#cb83f8"
            }

            StyleTypeButton
            {
                ButtonGroup.group: _styleGroup

                text: i18n("Black")


                foregroundColor: "#fff"
                backgroundColor: "#000"
                highlightColor: "#fff"

                checked: control.manager.styleType === 4
                onClicked: control.manager.styleType = 4
            }

            StyleTypeButton
            {
                ButtonGroup.group: _styleGroup

                text: i18n("White")


                foregroundColor: "#000"
                backgroundColor: "#fff"
                highlightColor: "#000"

                checked: control.manager.styleType === 5
                onClicked: control.manager.styleType = 5
            }
        }
    }


    Maui.SectionGroup
    {
        title: i18n("Interface")

        Maui.SectionItem
        {
            label1.text: i18n("Units and Preferences")
            label2.text: i18n("Customize the look-and-feel of elements.")
            //            iconSource: "preferences-desktop-tweaks"

            ToolButton
            {
                checkable: true
                icon.name: "go-next"
                onToggled: control.stackView.push(_unitsPageComponent)
            }
        }

        Maui.SectionItem
        {
            label1.text: i18n("Window Controls Decorations")
            label2.text: i18n("Style and options.")
            visible: !Maui.Handy.isMobile
            //            iconSource: "preferences-desktop-theme-windowdecorations"
            ToolButton
            {
                checkable: true
                icon.name: "go-next"
                onToggled: control.stackView.push(_windowControlsPageComponent)
            }
        }

        Maui.SectionItem
        {
            //            iconSource: "preferences-desktop-font"
            label1.text: i18n("Fonts")
            label2.text: i18n("Pick the system fonts.")

            ToolButton
            {
                checkable: true
                icon.name: "go-next"
                onToggled: control.stackView.push(_fontsPageComponent)
            }
        }
    }

    Maui.SectionGroup
    {
        title: i18n("Colors")

        Maui.SectionItem
        {
            //            iconSource: "preferences-desktop-color"
            label1.text: i18n("Accent Color")
            label2.text: i18n("Custom accent color.")

            columns:1

            Maui.ColorsRow
            {
                Layout.fillWidth: true
                size: Maui.Style.iconSizes.medium*1.2
                colors: ["#26c6da", "#2C3E50", "#1976d2", "#212121", "#8e24aa", "#16A085", "#BBCDE5", "#E6BCC3", "#FEF9A7", "#00FFAB", "#2A2550", "#827397", "#FF6363", "#F900BF", "#FFF56D"]
                currentColor: control.manager.accentColor

                onColorPicked:
                {
                    control.manager.accentColor = color
                }
            }

            Maui.Chip
            {
                Layout.fillWidth: true

                visible: control.manager.styleType > 1
                text: i18n("Accent color is only used with the Light and Dark styles.")
                color: Maui.Theme.neutralBackgroundColor
                iconSource: "dialog-warning"
            }
        }

        Maui.SectionItem
        {
            label1.text: i18n("Color Scheme")
            label2.text: i18n("Pick a custom color scheme.")
            enabled: control.manager.styleType === 3

            ToolButton
            {
                checkable: true
                icon.name: "go-next"
                onToggled: control.stackView.push(_colorSchemesPageComponent)
            }

            Maui.Chip
            {
                Layout.fillWidth: true

                visible: control.manager.styleType !== 3
                text: i18n("Enable the Custom style to be able to pick different schemes.")
                color: Maui.Theme.neutralBackgroundColor
                iconSource: "dialog-warning"
            }
        }
    }

    Maui.SectionGroup
    {
        title: i18n("Icons")

        Maui.SectionItem
        {
            label1.text: i18n("Icon Size")
            label2.text: i18n("Custom button icon sizes.")
            columns : 3

            Maui.ToolActions
            {
                autoExclusive: true
                expanded: true

                Action
                {
                    text: i18n("S")
                    checked: control.manager.iconSize === 16
                    onTriggered: control.manager.iconSize = 16
                }

                Action
                {
                    text: i18n("M")
                    checked: control.manager.iconSize === 22
                    onTriggered: control.manager.iconSize = 22
                }

                Action
                {
                    text: i18n("L")
                    checked: control.manager.iconSize === 32
                    onTriggered: control.manager.iconSize = 32
                }
            }

            Button
            {
                text: i18n("Reset")
                onClicked: control.manager.iconSize = undefined
            }
        }

        Maui.SectionItem
        {
            label1.text: i18n("Icon Theme")
            label2.text: control.manager.iconTheme
            //            iconSource: "preferences-desktop-icons"

            ToolButton
            {
                checkable: true
                icon.name: "go-next"
                onToggled: control.stackView.push(_iconsPageComponent)
            }
        }
    }
}
