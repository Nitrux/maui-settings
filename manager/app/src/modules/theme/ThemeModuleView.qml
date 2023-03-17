// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2

import org.mauikit.controls 1.3 as Maui

import org.maui.settings.lib 1.0 as ManLib

ManLib.SettingsPage
{
    id: control

    property var manager : control.module.manager

    Component
    {
        id: _iconsPageComponent


        Maui.GridBrowser
        {
            property string title: i18n("Icon Theme")

            property Component searchFieldComponent  :  Maui.SearchField
            {
                placeholderText: i18n ("Filter")
                onAccepted: control.manager.iconsModel.filter = text
                onCleared: control.manager.iconsModel.filter = undefined
            }

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

                    template.iconComponent: GridLayout
                    {
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

                    onClicked: control.manager.iconTheme = model.themeName

                    label1.text: model.display
                }

            }
        }


    }

    Component
    {
        id: _unitsPageComponent
        Maui.SettingsPage
        {
            title: i18n("Units")
            Maui.SectionItem
            {
                label1.text: i18n("Effects")
                label2.text: i18n("Enable effects.")

                Switch
                {
                    checked: control.manager.enableEffects
                    onToggled: control.manager.enableEffects = !control.manager.enableEffects
                }
            }

            Maui.SectionItem
            {
                label1.text: i18n("Border Radius")
                label2.text: i18n("Custom border radius.")
                columns : 3

                SpinBox
                {
                    from: 0
                    to: 20
                    value: control.manager.borderRadius
                    stepSize: 1
                    onValueModified: control.manager.borderRadius = value
                }

                Button
                {
                    text: i18n("Reset")
                    onClicked: control.manager.borderRadius = undefined
                }
            }


            Maui.SectionItem
            {
                label1.text: i18n("Elements Size")
                label2.text: i18n("Size of the UI elements.")
                columns : 3

                Maui.ToolActions
                {
                    autoExclusive: true
                    expanded: true

                    Action
                    {
                        text: i18n("S")
                        checked: control.manager.paddingSize === 4
                        onTriggered: control.manager.paddingSize = 4
                    }

                    Action
                    {
                        text: i18n("M")
                        checked: control.manager.paddingSize === 6
                        onTriggered: control.manager.paddingSize = 6
                    }

                    Action
                    {
                        text: i18n("L")
                        checked: control.manager.paddingSize === 8
                        onTriggered: control.manager.paddingSize = 8
                    }
                }

                Button
                {
                    text: i18n("Reset")
                    onClicked: control.manager.paddingSize = undefined
                }
            }

            Maui.SectionItem
            {
                label1.text: i18n("Content Margins")
                label2.text: i18n("Margins of the views.")
                columns : 3

                Maui.ToolActions
                {
                    autoExclusive: true
                    expanded: true

                    Action
                    {
                        text: i18n("S")
                        checked: control.manager.marginSize === 4
                        onTriggered: control.manager.marginSize = 4
                    }

                    Action
                    {
                        text: i18n("M")
                        checked: control.manager.marginSize === 6
                        onTriggered: control.manager.marginSize = 6
                    }

                    Action
                    {
                        text: i18n("L")
                        checked: control.manager.marginSize === 8
                        onTriggered: control.manager.marginSize = 8
                    }
                }

                Button
                {
                    text: i18n("Reset")
                    onClicked: control.manager.marginSize = undefined
                }
            }

            Maui.SectionItem
            {
                label1.text: i18n("Content Spacing")
                label2.text: i18n("Spacing between elements, such as list, tab buttons.")
                columns : 3

                Maui.ToolActions
                {
                    autoExclusive: true
                    expanded: true

                    Action
                    {
                        text: i18n("S")
                        checked: control.manager.spacingSize === 4
                        onTriggered: control.manager.spacingSize = 4
                    }

                    Action
                    {
                        text: i18n("M")
                        checked: control.manager.spacingSize === 6
                        onTriggered: control.manager.spacingSize = 6
                    }

                    Action
                    {
                        text: i18n("L")
                        checked: control.manager.spacingSize === 8
                        onTriggered: control.manager.spacingSize = 8
                    }
                }

                Button
                {
                    text: i18n("Reset")
                    onClicked: control.manager.spacingSize = undefined
                }
            }
        }
    }


    Component
    {
        id: _windowControlsPageComponent
        Maui.SettingsPage
        {
            title: i18n("Window Controls")
            Maui.SectionItem
            {
                label1.text: i18n("Style")
                label2.text: i18n("Enable CLient Side Decorations for MauiApps.")
                columns : 1

                Flow
                {
                    Layout.fillWidth: true
                    spacing: Maui.Style.space.medium

                    Repeater
                    {
                        model: control.manager.windowDecorationsModel

                        delegate: Item
                        {
                            width: _layoutWD.implicitWidth
                            height: _layoutWD.implicitHeight

                            Column
                            {
                                id: _layoutWD
                                spacing: Maui.Style.space.medium

                                Control
                                {
                                    Maui.Theme.colorSet: Maui.Theme.Header
                                    Maui.Theme.inherit: false
                                    padding: Maui.Style.space.medium
                                    background: Rectangle
                                    {
                                        radius: Maui.Style.radiusV
                                        color: Maui.Theme.backgroundColor
                                    }

                                    contentItem: Loader
                                    {
                                        property bool isActiveWindow: true
                                        property bool maximized: false
                                        property var buttonsModel : Maui.App.controls.rightWindowControls
                                        asynchronous: true
                                        source: model.source
                                    }
                                }

                                Button
                                {
                                    width: parent.width
                                    checked: model.name === Maui.App.controls.styleName
                                    text: model.name
                                    onClicked: control.manager.windowControlsTheme = model.name
                                }
                            }

                            RadioButton
                            {
                                autoExclusive: true
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.margins: Maui.Style.space.medium

                                visible: model.name === Maui.App.controls.styleName
                                checked: visible
                            }
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                label1.text: i18n("Use CSD")
                label2.text: i18n("Enable CLient Side Decorations for MauiApps.")

                Switch
                {
                    checked: control.manager.enableCSD
                    onToggled: control.manager.enableCSD = !control.manager.enableCSD
                }
            }

        }
    }

    Component
    {
        id: _colorSchemesPageComponent

        Maui.GridBrowser
        {
            readonly property string title: "Color Schemes"

            property Component searchFieldComponent:  Maui.SearchField
            {
                placeholderText: i18n ("Filter")
                onAccepted: control.manager.colorSchemeModel.filter = text
                onCleared: control.manager.colorSchemeModel.filter = undefined
            }

            model: control.manager.colorSchemeModel

            itemSize: 120
            itemHeight: 140

            delegate: Maui.GridBrowserDelegate
            {
                id: _delegate
                height: GridView.view.cellHeight
                width: GridView.view.itemSize
                checked: model.file === control.manager.customColorScheme
                onClicked:  control.manager.customColorScheme = model.file

                readonly property var colors : model.preview

                template.iconComponent: Control
                {
                    implicitHeight: Math.max(_layout.implicitHeight + topPadding + bottomPadding, 64)
                    padding: Maui.Style.space.small

                    background: Rectangle
                    {
                        color: _delegate.colors[0] //first color is the window background color
                        radius: Maui.Style.radiusV
                    }

                    contentItem: Column
                    {
                        id:_layout
                        spacing: 2

                        Label
                        {
                            width: parent.width
                            text: i18n ("Hello")
                            color: _delegate.colors[1]
                        }

                        Rectangle
                        {
                            radius: 2
                            height: 16
                            width: parent.width
                            color: _delegate.colors[2]
                        }

                        Rectangle
                        {
                            radius: 2
                            height: 6
                            width: parent.width
                            color: _delegate.colors[4]
                        }

                        Rectangle
                        {
                            radius: 2
                            height: 26
                            width: parent.width
                            color: _delegate.colors[3]
                        }

                    }
                }

                label1.text: model.name
            }
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

            ManLib.GraphicButton
            {
                text: i18n("Light")
                checked: control.manager.styleType === 0
                onClicked: control.manager.styleType = 0
                ButtonGroup.group: _styleGroup

                iconComponent: Control
                {
                    implicitHeight: Math.min(Screen.desktopAvailableHeight * 0.1, 200)
                    implicitWidth: Math.min(Screen.desktopAvailableWidth * 0.1, 200)

                    padding: Maui.Style.space.medium

                    background: Rectangle
                    {
                        color: Maui.Theme.backgroundColor
                        radius: Maui.Style.radiusV
                        Maui.IconItem
                        {
                            anchors.fill: parent

                            imageSource:  control.manager.wallpaper
                            fillMode: Image.PreserveAspectCrop
                            maskRadius: 10
                        }
                    }

                    contentItem: ColumnLayout
                    {
                        Button
                        {
                            Layout.fillWidth: true
                            icon.name: "love"
                            icon.color: "#333"

                            background: Rectangle
                            {
                                radius: 10
                                color: "#fafafa"
                                opacity: 0.5
                            }
                        }

                        Item
                        {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }

                        Rectangle
                        {
                            color: control.manager.accentColor

                            Layout.fillWidth: true
                            Layout.maximumHeight: 24
                            Layout.preferredHeight: 24

                            radius: 10
                        }

                        Rectangle
                        {
                            color: "#fafafa"

                            Layout.fillWidth: true
                            Layout.maximumHeight: 64
                            Layout.preferredHeight: 48
                            radius: 10
                        }
                    }
                }
            }

            ManLib.GraphicButton
            {
                text: i18n("Dark")
                checked: control.manager.styleType === 1
                onClicked: control.manager.styleType = 1
                ButtonGroup.group: _styleGroup
                iconComponent: Control
                {
                    implicitHeight: Math.min(Screen.desktopAvailableHeight * 0.1, 200)
                    implicitWidth: Math.min(Screen.desktopAvailableWidth * 0.1, 200)

                    padding: Maui.Style.space.medium

                    background: Rectangle
                    {
                        color: Maui.Theme.backgroundColor
                        radius: Maui.Style.radiusV
                        Maui.IconItem
                        {
                            anchors.fill: parent

                            imageSource:  control.manager.wallpaper
                            fillMode: Image.PreserveAspectCrop
                            maskRadius: 10
                        }
                    }

                    contentItem: ColumnLayout
                    {
                        Button
                        {
                            Layout.fillWidth: true
                            icon.name: "love"
                            icon.color: "#fafafa"

                            background: Rectangle
                            {
                                radius: 10
                                color: "#333"
                                opacity: 0.5
                            }
                        }

                        Item
                        {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }

                        Rectangle
                        {
                            color: control.manager.accentColor

                            Layout.fillWidth: true
                            Layout.maximumHeight: 24
                            Layout.preferredHeight: 24

                            radius: 10
                        }

                        Rectangle
                        {
                            color: "#333"

                            Layout.fillWidth: true
                            Layout.maximumHeight: 64
                            Layout.preferredHeight: 48

                            radius: 10
                        }
                    }
                }
            }

            ManLib.GraphicButton
            {
                text: i18n("Adative")
                checked: control.manager.styleType === 2
                onClicked: control.manager.styleType = 2
                ButtonGroup.group: _styleGroup
                iconComponent: Control
                {
                    implicitHeight: Math.min(Screen.desktopAvailableHeight * 0.1, 200)
                    implicitWidth: Math.min(Screen.desktopAvailableWidth * 0.1, 200)

                    padding: Maui.Style.space.medium

                    Maui.ImageColors
                        {
                            id: _imgColors
                            source: control.manager.wallpaper.replace("file://", "")
                            onPaletteChanged:
                            {
                                console.log(_imgColors.average)
                            }
                        }



                    background: Rectangle
                    {
                        color: Maui.Theme.backgroundColor
                        radius: Maui.Style.radiusV
                        Maui.IconItem
                        {
                            id: _iconWallpaper
                            anchors.fill: parent

                            imageSource:  control.manager.wallpaper
                            fillMode: Image.PreserveAspectCrop
                            maskRadius: 10
                        }
                    }

                    contentItem: ColumnLayout
                    {
                        Button
                        {
                            Layout.fillWidth: true
                            icon.name: "love"
                            icon.color: _imgColors.foreground

                            background: Rectangle
                            {
                                radius: 10
                                color: _imgColors.background
                                opacity: 0.5
                            }
                        }

                        ItemHa
                        {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }

                        Rectangle
                        {
                            color:  _imgColors.highlight

                            Layout.fillWidth: true
                            Layout.maximumHeight: 24
                            Layout.preferredHeight: 24

                            radius: 10
                        }

                        Rectangle
                        {
                            color: _imgColors.average

                            Layout.fillWidth: true
                            Layout.maximumHeight: 64
                            Layout.preferredHeight: 48
                            radius: 10
                        }
                    }
                }
            }

            ManLib.GraphicButton
            {
                text: i18n("Custom")
                checked: control.manager.styleType === 3
                onClicked: control.manager.styleType = 3
                ButtonGroup.group: _styleGroup
                iconComponent: Control
                {
                    implicitHeight: Math.min(Screen.desktopAvailableHeight * 0.1, 200)
                    implicitWidth: Math.min(Screen.desktopAvailableWidth * 0.1, 200)

                    padding: Maui.Style.space.medium

                    background: Rectangle
                    {
                        color: Maui.Theme.backgroundColor
                        radius: Maui.Style.radiusV
                        Maui.IconItem
                        {
                            anchors.fill: parent

                            imageSource:  control.manager.wallpaper
                            fillMode: Image.PreserveAspectCrop
                            maskRadius: 10
                        }
                    }

                    contentItem: ColumnLayout
                    {
                        Button
                        {
                            Layout.fillWidth: true
                            icon.name: "love"

                            background: Rectangle
                            {
                                radius: 10
                                color: Maui.Theme.backgroundColor
                                opacity: 0.5
                            }
                        }

                        Item
                        {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }

                        Rectangle
                        {
                            color: Maui.Theme.highlightColor
                            Layout.fillWidth: true
                            Layout.maximumHeight: 24
                            Layout.preferredHeight: 24

                            radius: 10
                        }

                        Rectangle
                        {
                            color: Maui.Theme.backgroundColor

                            Layout.fillWidth: true
                            Layout.maximumHeight: 64
                            Layout.preferredHeight: 48

                            radius: 10
                        }
                    }
                }
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
            ToolButton
            {
                checkable: true
                icon.name: "go-next"
                onToggled: control.stackView.push(_windowControlsPageComponent)
            }
        }

        Maui.SectionItem
        {
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
            ToolButton
            {
                checkable: true
                icon.name: "go-next"
                onToggled: control.stackView.push(_iconsPageComponent)
            }
        }
    }


    Component
    {
        id: _fontsPageComponent
        Maui.SettingsPage
        {
            title: i18n ("Fonts")

            Maui.SectionGroup
            {
                title: i18n("System Fonts")
                description: i18n ("Pick the default system fonts to be used.")

                Maui.SectionItem
                {
                    id: _defaultFontSection
                    label1.text: i18n("Default Font")
                    label2.text: defaultFont.family
                    columns: 3
                    property font defaultFont : control.module.getFont(control.manager.defaultFont)

                    Button
                    {
                        text: _defaultFontSection.defaultFont.family
                        onClicked:
                        {
                            _fontEditDialog.mfont = control.module.getFont(control.manager.defaultFont)
                            _fontEditDialog.open()
                        }
                    }

                    Button
                    {
                        text: i18n("Reset")
                        onClicked: control.manager.defaultFont = undefined
                    }
                }
            }

            Maui.FontPickerDialog
            {
                id: _fontEditDialog

                onAccepted:
                {
                    let desc = control.module.fontToString(_fontEditDialog.mfont)
                    control.manager.defaultFont = desc
                }

            }
        }
    }


}
