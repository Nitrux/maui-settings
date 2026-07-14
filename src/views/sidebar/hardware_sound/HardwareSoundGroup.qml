import QtQuick

Item
{
    width: 0
    height: 0
    visible: false
    property string title: i18n("Hardware & Sound")
    property var items: []

    Component
    {
        id: audioComponent
        AudioItem {}
    }

    Component
    {
        id: inputDevicesComponent
        InputDevicesItem {}
    }

    Component
    {
        id: formFactorComponent
        FormFactorItem {}
    }

    Component
    {
        id: accessibilityComponent
        AccessibilityItem {}
    }

    Component.onCompleted:
    {
        items = [
            audioComponent.createObject(this),
            inputDevicesComponent.createObject(this),
            formFactorComponent.createObject(this),
            accessibilityComponent.createObject(this)
        ]
    }
}
