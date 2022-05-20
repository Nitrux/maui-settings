import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.14

import org.mauikit.controls 1.3 as Maui

import "src/controls"

Maui.ApplicationWindow
{
    id: root
    title: qsTr("Maui Manager")
headBar.visible: false

    HomeView
    {
        id: _homeView
        anchors.fill: parent
    }
}
