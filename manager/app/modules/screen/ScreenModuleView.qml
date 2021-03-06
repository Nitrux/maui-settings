// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui

import org.maui.settings 1.0 as Man
import org.maui.settings.lib 1.0 as ManLib

ManLib.SettingsPage
{
    id: control

    property var module
    property var manager : control.module.manager
    title: module.name


    Maui.SettingsSection
    {
        title: i18n("Options")
        description: i18n("Screen configurations.")

        Maui.SettingTemplate
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
