// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

import org.maui.settings.lib as ManLib

Maui.SettingsPage
{
    id: control

    property var manager

    title: i18n ("Fonts")

    Maui.SectionGroup
    {
        title: i18n("System Fonts")
        description: i18n ("Pick the default system fonts to be used.")

        Maui.FlexSectionItem
        {
            id: _defaultFontSection
            label1.text: i18n("Default Font")
            label2.text: defaultFont.family
            property font defaultFont : module.getFont(control.manager.defaultFont)

            Button
            {
                text: _defaultFontSection.defaultFont.family
                onClicked:
                {
                    _fontEditDialog.mfont = module.getFont(control.manager.defaultFont)
                    _fontEditDialog.model.onlyMonospaced = false
                    _fontEditDialog.callback = function(desc)
                    {
                        control.manager.defaultFont = desc
                    }
                    _fontEditDialog.open()

                }
            }

            Button
            {
                text: i18n("Reset")
                onClicked: control.manager.defaultFont = undefined
            }
        }

        Maui.FlexSectionItem
        {
            id: _smallFontSection
            label1.text: i18n("Small Font")
            label2.text: smallFont.family
            property font smallFont : module.getFont(control.manager.smallFont)

            Button
            {
                text: _smallFontSection.smallFont.family
                onClicked:
                {
                    _fontEditDialog.mfont = module.getFont(control.manager.smallFont)
                    _fontEditDialog.model.onlyMonospaced = false
                    _fontEditDialog.callback = function(desc)
                    {
                        control.manager.smallFont = desc
                    }

                    _fontEditDialog.open()
                }
            }

            Button
            {
                text: i18n("Reset")
                onClicked: control.manager.smallFont = undefined
            }
        }

        Maui.FlexSectionItem
        {
            id: _monospacedFontSection
            label1.text: i18n("Monospaced Font")
            label2.text: monospacedFont.family
            property font monospacedFont : module.getFont(control.manager.monospacedFont)

            Button
            {
                text: _monospacedFontSection.monospacedFont.family
                onClicked:
                {
                    _fontEditDialog.mfont = module.getFont(control.manager.monospacedFont)
                    _fontEditDialog.model.onlyMonospaced = true
                    _fontEditDialog.callback = function(desc)
                    {
                        control.manager.monospacedFont = desc
                    }

                    _fontEditDialog.open()
                }
            }

            Button
            {
                text: i18n("Reset")
                onClicked: control.manager.monospacedFont = undefined
            }
        }
    }

    Maui.FontPickerDialog
    {
        id: _fontEditDialog

        property var callback

        onAccepted:
        {
            if(_fontEditDialog.callback)
            {
                let desc = module.fontToString(_fontEditDialog.mfont)
                _fontEditDialog.callback(desc)
            }

            _fontEditDialog.close()
        }
    }
}

