import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.14

import org.mauikit.controls 1.3 as Maui
import org.nxmanager.touchegg 1.0 as  T

Maui.ApplicationWindow
{
    id: root
    title: qsTr("Hello World")

    headBar.leftContent: Maui.ToolButtonMenu
    {
        icon.name: "application-menu"


        MenuItem
        {
            text: i18n("Settings")
            icon.name: "settings-configure"
            onTriggered:
            {
                _dialogLoader.sourceComponent = _settingsDialogComponent
                dialog.open()
            }
        }

        MenuItem
        {
            text: i18n("About")
            icon.name: "documentinfo"
            onTriggered: root.about()
        }
    }

    headBar.middleContent: Maui.TextField
    {
        placeholderText: i18n("Search configs")
    }


    T.Touchegg
    {
        anchors.fill: parent
    }
}
