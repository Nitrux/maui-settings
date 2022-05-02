import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12

import org.mauikit.controls 1.3 as Maui
import org.nxmanager.touchegg 1.0 as  T

import "src/controls"

Maui.ApplicationWindow
{
    id: root
    title: qsTr("Maui Manager")

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
        Layout.alignment: Qt.AlignCenter
        Layout.maximumWidth: 500
        Layout.fillWidth: true
        placeholderText: i18n("Search configs")
    }

    HomeView
    {
        anchors.fill: parent
    }

}
