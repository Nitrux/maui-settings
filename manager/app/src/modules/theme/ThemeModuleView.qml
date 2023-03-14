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

    Component
    {
        id: _iconsPageComponent

        Maui.ScrollColumn
        {
            Maui.SectionItem
            {
                label1.text: i18n("Theme")
                label2.text: i18n("Pick the icon theme.")
                columns: 1

                Maui.GridBrowser
                {

                    Layout.fillWidth: true

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

        Maui.SectionItem
        {
            label1.text: i18n("Light")
            label2.text: i18n("Light colors tinted by the accent color.")
            template.imageSource: "qrc:/light_mode.png"

            RadioButton
            {
                //                autoExclusive: true
                checked: control.manager.styleType === 0
                ButtonGroup.group: _styleGroup
                onToggled: control.manager.styleType = 0
            }
        }

        Maui.SectionItem
        {
            label1.text: i18n("Dark")
            label2.text: i18n("Dark colors tinted by the accent color.")
            template.imageSource: "qrc:/dark_mode.png"

            RadioButton
            {
                //                autoExclusive: true
                checked: control.manager.styleType === 1
                ButtonGroup.group: _styleGroup
                onToggled: control.manager.styleType = 1
            }
        }

        Maui.SectionItem
        {
            label1.text: i18n("Adaptive")
            label2.text: i18n("Light colors tinted by the accent color.")
            template.imageSource: "qrc:/adaptive_mode.png"

            RadioButton
            {
                autoExclusive: true
                checked: control.manager.styleType === 2
                ButtonGroup.group: _styleGroup
                onToggled: control.manager.styleType = 2
            }
        }

        Maui.SectionItem
        {
            label1.text: i18n("Auto")
            label2.text: i18n("Light colors tinted by the accent color.")
            template.imageSource: "qrc:/auto_mode.png"

            RadioButton
            {
                autoExclusive: true
                checked: control.manager.styleType === 3
                ButtonGroup.group: _styleGroup
                onToggled: control.manager.styleType = 3
            }
        }
    }

    Maui.SectionGroup
    {
        title: i18n("Units and Preferences")
        description: i18n("Customize the look-and-feel of elements.")

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
        }

        Maui.Chip
        {
            visible: control.manager.styleType === 2
            text: i18n("Accent color is not used with the Adaptive Style.")
            color: Maui.Theme.neutralBackgroundColor
            iconSource: "dialog-warning"
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
            label1.text: i18n("Theme")
            label2.text: i18n("Pick the icon theme.")
            ToolButton
            {
                checkable: true
                icon.name: "go-next"
                onToggled: control.stackView.push(_iconsPageComponent)
            }
        }
    }

    Maui.SectionGroup
    {
        title: i18n("Window Control Decorations")
        visible: !Maui.Handy.isMobile

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
        }
    }

    Maui.Dialog
    {
        id: _fontEditDialog
        property font mfont : control.module.getFont(control.manager.defaultFont)
        property var styles : control.module.fontStyles(mfont)
        property var sizes: control.module.fontPointSizes(mfont)

        Maui.FontsComboBox
        {
            Layout.fillWidth: true
            Component.onCompleted: currentIndex = find(_fontEditDialog.mfont.family, Qt.MatchExactly)
            onActivated:
            {
                let newFont = _fontEditDialog.mfont
                newFont.family = currentText


                _fontEditDialog.mfont = newFont
                //                _fontEditDialog.styles = control.module.fontStyles(_fontEditDialog.mfont)
                //                _fontEditDialog.sizes = control.module.fontPointSizes(_fontEditDialog.mfont)
            }
        }

        ComboBox
        {
            Layout.fillWidth: true
            model: _fontEditDialog.styles
            Component.onCompleted: currentIndex = find(_fontEditDialog.mfont.styleName, Qt.MatchExactly)
            icon.source: "format-text-color"
            onActivated:
            {
                let newFont = _fontEditDialog.mfont
                newFont.styleName = currentText


                _fontEditDialog.mfont = newFont
            }
        }

        ComboBox
        {
            Layout.fillWidth: true
            model: _fontEditDialog.sizes
            Component.onCompleted: currentIndex = find(_fontEditDialog.mfont.pointSize, Qt.MatchExactly)
            icon.source: "font-size-down"
            onActivated:
            {
                let newFont = _fontEditDialog.mfont
                newFont.pointSize = currentText


                _fontEditDialog.mfont = newFont
            }
        }

        TextArea
        {
            Layout.fillWidth: true
            implicitHeight: contentHeight + topPadding + bottomPadding

            text: i18n("The Quick Brown Fox Jumps Over The Lazy Dog")
            font: _fontEditDialog.mfont
        }

        onAccepted:
        {
            let desc = control.module.fontToString(_fontEditDialog.mfont)
            control.manager.defaultFont = desc
        }

        onRejected:
        {
            _fontEditDialog.close()
        }
    }
}
