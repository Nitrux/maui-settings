// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import QtGraphicalEffects 1.0
import QtQuick.Dialogs 1.3

import org.mauikit.controls 1.3 as Maui

import org.maui.settings.lib 1.0 as ManLib

ManLib.SettingsPage
{
    id: control

    property var manager : control.module.manager

    Maui.SectionGroup
    {
        title: i18n("Navigation")
        description: i18n("User experience interactions")

        Maui.SectionItem
        {
            label1.text: i18n("Single Click")
            label2.text: i18n("Open items with a single click, otherwise with a double click.")

            Switch
            {
                checked: control.manager.singleClick
                onToggled: control.manager.singleClick = !control.manager.singleClick
            }
        }

    }

 }
