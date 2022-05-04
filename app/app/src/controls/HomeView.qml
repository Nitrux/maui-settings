import QtQuick 2.14
import QtQuick.Controls 2.14


import org.kde.kirigami 2.7 as Kirigami
import org.mauikit.controls 1.3 as Maui

import org.maui.manager 1.0 as Man

StackView
{
    id: control

    initialItem: Maui.ListBrowser
    {

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

    function loadModule(module)
    {
        console.log(module.qmlSource)
        control.push(module.qmlSource)
    }
}
