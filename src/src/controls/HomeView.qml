import QtQuick 2.14
import QtQuick.Controls 2.14


import org.kde.kirigami 2.7 as Kirigami
import org.mauikit.controls 1.3 as Maui

StackView
{
    id: control

    initialItem: Maui.ListBrowser
    {

        model : ["Background", "Theme", "Information", "Display", "Mouse", "Sound", "Network"]

        delegate: Maui.ListBrowserDelegate
        {
            width: ListView.view.width
            label1.text: modelData
            iconSource: "love"

        }
    }
}
