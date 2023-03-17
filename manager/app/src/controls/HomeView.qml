import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12

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
            holder.visible: count === 0
            holder.title:i18n("No Modules!")
            holder.body: i18n("No modules avaliable!")
            holder.emoji: "face-confused-symbolic"

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

            flickable.header: Column
            {
                width: parent.width
                spacing: Maui.Style.defaultSpacing

                Maui.Chip
                {
                    width: parent.width
                    height: visible ? implicitHeight : 0

                    visible: !ModulesManager.isMauiSession
                    text: i18n("Unable to detect a Cask session. The current session is %1", ModulesManager.currentDesktopSession)
                    ToolTip.text: i18n("Most of the settings won't be applied correctly or won't work as expected. For complete support start a Maui session.")
                    color: Maui.Theme.neutralBackgroundColor
                    iconSource: "dialog-warning"
                }

                Maui.Chip
                {
                    width: parent.width
                    height: visible ? implicitHeight : 0

                    visible: !ModulesManager.serverRunning
                    text:i18n("MauiMan server is offline.")
                    ToolTip.text: i18n("Changes in the setting preferences won't be applied live to other apps if the server is off. Changes will take effcet after restarting the apps.")
                    color: Maui.Theme.neutralBackgroundColor
                    iconSource: "dialog-warning"
                    onClicked: ModulesManager.startMauiManServer()
                }

                Maui.Chip
                {
                    width: parent.width
                    height: visible ? implicitHeight : 0

                    visible: !ModulesManager.caskServerRunning
                    text: i18n("Cask server is offline.")
                    ToolTip.text: i18n("Changes releated to Cask won't be updated live unless the server is running.")
                    color: Maui.Theme.neutralBackgroundColor
                    iconSource: "dialog-warning"
                    onClicked: ModulesManager.startCaskServer()
                }
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
        if(_viewLoader.currentItem  && module === _viewLoader.currentItem.module)
        {
            console.log("Same module requested. Do nothing", module)
            return;
        }

        _viewLoader.pop()
        _viewLoader.push(module.qmlSource, ({'module': module}))
    }

    function toggleSideBar()
    {
        control.sideBar.toggle()
    }
}
