import QtQuick

Item
{
    width: 0
    height: 0
    visible: false
    property string title: i18n("Desktop & Shell")
    property var items: []

    Component
    {
        id: desktopComponent
        DesktopItem {}
    }

    Component
    {
        id: panelComponent
        PanelItem {}
    }

    Component.onCompleted:
    {
        items = [
            desktopComponent.createObject(this),
            panelComponent.createObject(this)
        ]
    }
}
