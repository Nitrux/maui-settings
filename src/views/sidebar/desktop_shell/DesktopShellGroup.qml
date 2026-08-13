import QtQuick

Item
{
    width: 0
    height: 0
    visible: false
    property string title: i18n("Desktop")
    property var items: []

    Component
    {
        id: valenzComponent
        ValenzItem {}
    }

    Component
    {
        id: nudgeOsdComponent
        NudgeOsdItem {}
    }

    Component
    {
        id: marinaComponent
        MarinaItem {}
    }

    Component
    {
        id: windowCompositorComponent
        WindowCompositorItem {}
    }

    Component.onCompleted:
    {
        items = [
            valenzComponent.createObject(this),
            nudgeOsdComponent.createObject(this),
            marinaComponent.createObject(this),
            windowCompositorComponent.createObject(this)
        ]
    }
}
