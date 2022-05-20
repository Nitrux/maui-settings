import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.7 as Kirigami
import org.mauikit.controls 1.3 as Maui

import org.maui.manager 1.0 as Man

Maui.StackView
{
    id: control

    initialItem: Maui.Page
    {
        showCSDControls: true

        headBar.leftContent: [
            Maui.ToolButtonMenu
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
        ]

        headBar.middleContent: Maui.TextField
        {
            Layout.alignment: Qt.AlignCenter
            Layout.maximumWidth: 500
            Layout.fillWidth: true
            placeholderText: i18n("Search configs")
        }

        Maui.ListBrowser
        {
            anchors.fill: parent
            model : ModulesManager.model
            currentIndex : -1
            section.delegate: Maui.LabelDelegate {
                width: ListView.view.width
                label: section
                isSection: true
            }

            section.property: "Module.category"

            delegate: Maui.ListBrowserDelegate
            {
                width: ListView.view.width
                label1.text: Module.name
                label2.text: Module.description
                iconSource: Module.iconName

                onClicked:
                {
                    console.log(Module.qmlSource)
                    control.loadModule(Module)
                }
            }
        }
    }

    function loadModule(module)
    {
        console.log(module.qmlSource)
        control.push(module.qmlSource, ({'module': module}))
    }
}
