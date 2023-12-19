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

    function isModuleOpen(module)
    {
        return _homeView.currentModule === module
    }

    function openModule(module)
    {
        _homeView.loadModule(ModulesManager.sourceFor(module))
    }

}
