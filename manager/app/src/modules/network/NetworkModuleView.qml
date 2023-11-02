// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick

import org.mauikit.controls as Maui
import org.maui.settings.lib as ManLib

ManLib.SettingsPage
{
    id: control

    property var manager : control.module.manager

    Maui.SectionGroup
    {
        title: i18n("Fonts")
    }
}
