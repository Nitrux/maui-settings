import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import org.mauikit.filebrowsing 1.3 as FB

import org.mauikit.controls 1.3 as Maui

Maui.SideBarView
{
    id: control

    property string currentModule : _viewLoader.currentItem.title
    sideBar.minimumWidth: Maui.Style.units.gridUnit * 14
    sideBar.preferredWidth: Maui.Style.units.gridUnit * 18
    sideBarContent: Maui.Page
    {
        anchors.fill: parent
        //        showCSDControls: true

        //        headBar.leftContent: [
        //            Maui.ToolButtonMenu
        //            {
        //                icon.name: "application-menu"

        //                MenuItem
        //                {
        //                    text: i18n("Settings")
        //                    icon.name: "settings-configure"
        //                    onTriggered:
        //                    {
        //                        _dialogLoader.sourceComponent = _settingsDialogComponent
        //                        dialog.open()
        //                    }
        //                }

        //                MenuItem
        //                {
        //                    text: i18n("About")
        //                    icon.name: "documentinfo"
        //                    onTriggered: root.about()
        //                }
        //            }
        //        ]

        headBar.middleContent: Maui.TextField
        {
            enabled: ModulesManager.serverRunning

            Layout.alignment: Qt.AlignCenter
            Layout.maximumWidth: 500
            Layout.fillWidth: true
            placeholderText: i18n("Search configs")
        }

        Maui.ListBrowser
        {
            anchors.fill: parent
            //            enabled: ModulesManager.serverRunning
            holder.visible: count === 0 || !ModulesManager.serverRunning
            holder.title: !ModulesManager.serverRunning ? i18n("Not server!") : i18n("No Modules!")
            holder.body: !ModulesManager.serverRunning ? i18n("MauiMan server is not running") : i18n("No modules avaliable!")
            holder.emoji: "face-confused-symbolic"
            holder.actions: Action
            {
                text: i18n("Start server")
                onTriggered: ModulesManager.startServer()
            }

            model : ModulesManager.model
            currentIndex : -1
            section.delegate: Maui.LabelDelegate
            {
                width: ListView.view.width
                label: section
                isSection: true
            }

            section.property: "Module.category"

            delegate: Maui.ListBrowserDelegate
            {
                width: ListView.view.width
                isCurrentItem: Module.name === control.currentModule
                label1.text: Module.name
                label2.text: Module.description
                iconSource: Module.iconName
                template.iconSizeHint: Maui.Style.iconSizes.medium
                onClicked:
                {
                    console.log(Module.qmlSource)
                    control.loadModule(Module)
                }
            }
        }
    }

    StackView
    {
        id: _viewLoader
        //        asynchronous: true
        anchors.fill: parent
    }

    Component.onCompleted:
    {
        console.log("ASK TO OPEN AT THE MODULE", initModule, ModulesManager.sourceFor(initModule))
        if(initModule.length)
            loadModule(ModulesManager.sourceFor(initModule))
        else
            loadModule(ModulesManager.sourceFor("About"))
    }

    function loadModule(module)
    {
        console.log(module.qmlSource)
        _viewLoader.push(module.qmlSource, ({'module': module}))
    }

    function toggleSideBar()
    {
        control.sideBar.toggle()
    }
}
