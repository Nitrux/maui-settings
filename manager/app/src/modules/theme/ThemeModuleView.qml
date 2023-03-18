// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui

import org.maui.settings.lib 1.0 as ManLib

ManLib.SettingsPage
{
    id: control

    property var manager : control.module.manager

    property int styleItemHeight: Math.min(control.height * 0.2, 200)
    property int styleItemWidth : Math.min(control.width * 0.2, 400)

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
        title: i18n("Style")
        description: i18n("Pick a style variant.")

        GridLayout
        {
            Layout.alignment: Qt.AlignCenter

            columns: parent.width > 600 ? 4 : 2
            columnSpacing: Maui.Style.space.big
            rowSpacing: Maui.Style.space.big

            StyleTypeButton
            {
                previewHeight: control.styleItemHeight
                previewWidth: control.styleItemWidth
                text: i18n("Light")
                checked: control.manager.styleType === 0
                onClicked: control.manager.styleType = 0
                imageSource: control.manager.wallpaper
                ButtonGroup.group: _styleGroup
                foregroundColor: "#333"
                backgroundColor: "#fafafa"
                highlightColor: control.manager.accentColor
            }

            StyleTypeButton
            {
                previewHeight: control.styleItemHeight
                previewWidth: control.styleItemWidth
                text: i18n("Dark")
                checked: control.manager.styleType === 1
                onClicked: control.manager.styleType = 1
                ButtonGroup.group: _styleGroup
                foregroundColor: "#fafafa"
                backgroundColor: "#333"
                highlightColor: control.manager.accentColor
                imageSource: control.manager.wallpaper

            }

            StyleTypeButton
            {
                previewHeight: control.styleItemHeight
                previewWidth: control.styleItemWidth
                text: i18n("Adaptive")
                checked: control.manager.styleType === 2
                onClicked: control.manager.styleType = 2
                ButtonGroup.group: _styleGroup
                foregroundColor: _imgColors.foreground
                backgroundColor: _imgColors.background
                highlightColor: _imgColors.average
                imageSource: control.manager.wallpaper

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
                previewHeight: control.styleItemHeight
                previewWidth: control.styleItemWidth

                text: i18n("Custom")
                checked: control.manager.styleType === 3
                onClicked: control.manager.styleType = 3
                ButtonGroup.group: _styleGroup
                foregroundColor: Maui.Theme.textColor
                backgroundColor: Maui.Theme.backgroundColor
                highlightColor:Maui.Theme.highlightColor
                imageSource: control.manager.wallpaper

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

            Maui.ColorsRow
            {
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
                visible: control.manager.styleType === 2 || control.manager.styleType === 3
                text: i18n("Accent color is not used with the Adaptive or Custom styles.")
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
