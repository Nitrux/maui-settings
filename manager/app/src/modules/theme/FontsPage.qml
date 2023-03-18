// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui

import org.maui.settings.lib 1.0 as ManLib

Maui.SettingsPage
{
    id: control

    property var manager

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

        Maui.SectionItem
        {
            id: _smallFontSection
            label1.text: i18n("Small Font")
            label2.text: smallFont.family
            columns: 3
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

        Maui.SectionItem
        {
            id: _monospacedFontSection
            label1.text: i18n("Monospaced Font")
            label2.text: monospacedFont.family
            columns: 3
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

