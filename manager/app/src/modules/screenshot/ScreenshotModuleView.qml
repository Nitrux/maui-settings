// Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
// Copyright 2018-2020 Nitrux Latinoamericana S.C.
//
// SPDX-License-Identifier: GPL-3.0-or-later


import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

import org.mauikit.controls 1.3 as Maui
import org.mauikit.filebrowsing 1.3 as FB
import org.maui.settings.lib 1.0 as ManLib

ManLib.SettingsPage
{
    id: control

    property var manager : control.module.manager

    Maui.NewDialog
    {
        id: _newBlacklistedDialog

        onFinished: control.manager.blacklist(text)
    }


    Maui.SectionGroup
    {
        title: i18n("Privacy")
        description: i18n("List of blacklisted app that won't be shown in screenshots.")

        template.template.content: ToolButton
        {
            icon.name: "list-add"
            onClicked: _newBlacklistedDialog.open()
        }

        Column
        {
            Layout.fillWidth: true

            Repeater
            {
                model: control.manager.blacklistedModel

                delegate: Maui.ListBrowserDelegate
                {
                    width: parent.width
                    label1.text: modelData.name
                    label2.text: modelData.comment
                    label3.text: modelData.id
                    iconSource: modelData.icon
                    enabled: modelData.token
                    Switch
                    {
                        checked: true
                    onToggled: control.manager.unblacklist(modelData.id)
                    }
                }
            }
        }
    }

    Maui.SectionGroup
    {
        title: i18n("Options")
        description: i18n("Configure screenshot behaviour.")

        Maui.SectionItem
        {
            label1.text: i18n("Save Directory")
            label2.text: i18n("Pick the default location where screenshots will be saved.")

            TextField
            {
                width: parent.parent.width
                placeholderText: i18n("New location")
                text: control.manager.saveDir

                onAccepted:
                {

                    if(FB.FM.fileExists(text))
                    control.manager.saveDir = text //make sure the entered text is an actual location and that it exists?
                }
            }
        }

    }
}
