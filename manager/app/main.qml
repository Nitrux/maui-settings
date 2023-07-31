import QtQuick
import QtQuick.Controls
import QtCore

import org.mauikit.controls as Maui

import "src/controls"

Maui.ApplicationWindow
{
    id: root
    title: _homeView.title

    Settings
    {
        id: settings
        property alias sideBarWidth : _homeView.sideBarWidth
    }

    HomeView
    {
        id: _homeView
        anchors.fill: parent
    }

    function isModuleOpen(module : String)
    {
        return _homeView.currentModule === module
    }

    function openModule(module : String)
    {
        _homeView.loadModule(ModulesManager.sourceFor(module))
    }

}
