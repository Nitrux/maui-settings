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

    Maui.SectionGroup
    {
        title: i18n("Options")
        description: i18n("Screen configurations.")

        Maui.SectionItem
        {
            label1.text: i18n("Scale Factor")
            label2.text: i18n("Make UI elements larger or smaller by scaling them.")

            RowLayout
            {
                width: parent.parent.width
                Slider
                {
                    id: _scaleSlider
                    Layout.fillWidth: true
                    stepSize: 0.25
                    from: 1
                    to : 3
                    value: control.manager.scaleFactor
                    onMoved:  control.manager.scaleFactor = value
                }

                Label
                {
                    Layout.fillHeight: true
                    font.bold: true
                    font.weight: Font.Bold
                    text: _scaleSlider.value
                }
            }
        }

    }

}
