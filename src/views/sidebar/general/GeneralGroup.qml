import QtQuick

Item
{
    width: 0
    height: 0
    visible: false
    property string title: i18n("General")
    property var items: []

    Component
    {
        id: aboutComponent
        AboutItem {}
    }

    Component
    {
        id: accessibilityComponent
        AccessibilityItem {}
    }

    Component
    {
        id: dateTimeComponent
        SystemItem {}
    }

    Component.onCompleted:
    {
        items = [
            aboutComponent.createObject(this),
            accessibilityComponent.createObject(this),
            dateTimeComponent.createObject(this)
        ]
    }
}
