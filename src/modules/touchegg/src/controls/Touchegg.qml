// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import org.mauikit.controls 1.3 as Maui

Rectangle
{
    id: control
    color: "orange"


    Maui.SettingsSection
    {
        anchors.fill: parent

        Maui.SettingTemplate
        {
            label1.text: i18n("Three fingers")

            Switch{}
        }
    }
}
