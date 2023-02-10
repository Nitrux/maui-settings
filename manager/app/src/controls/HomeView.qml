import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12
import org.mauikit.filebrowsing 1.3 as FB

import org.mauikit.controls 1.3 as Maui

Maui.SideBarView
{
    id: control

    property string currentModule : _viewLoader.currentItem.moduleId
    property string title : _viewLoader.currentItem.title
property alias sideBarWidth :  control.sideBar.preferredWidth

    sideBar.minimumWidth: 200
    sideBar.preferredWidth: 300
    sideBarContent: Maui.Page
    {
        anchors.fill: parent

Maui.Theme.colorSet: Maui.Theme.Window
Maui.Theme.inherit: false
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

        headBar.middleContent: TextField
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

            delegate: Maui.ListDelegate
            {
                width: ListView.view.width
                isCurrentItem: Module.id === control.currentModule
                label: Module.name
                //label2: Module.description
                iconName: Module.iconName
                template.iconSizeHint: Maui.Style.iconSizes.medium
                onClicked:
                {
                    console.log(Module.qmlSource)
                    control.loadModule(Module)
                }
            }

            flickable.header: Maui.Chip
                {
                    width: parent.width
height: visible ? implicitHeight : 0

                    visible: !ModulesManager.isMauiSession
                    text: i18n("A Maui session has not been detected. The session detected is %1", ModulesManager.currentDesktopSession)
ToolTip.text: i18n("Most of the settings won't be applied correctly or won't work as expected. For full and integrated support launch a full Maui session.")
                    color: Maui.Theme.neutralBackgroundColor
                    iconSource: "dialog-warning"
                }
        }
    }


        StackView
        {
            id: _viewLoader
            anchors.fill: parent

        }


    function loadModule(module)
    {
        _viewLoader.pop()
        _viewLoader.push(module.qmlSource, ({'module': module}))
    }

    function toggleSideBar()
    {
        control.sideBar.toggle()
    }
}
