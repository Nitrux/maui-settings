import QtQuick

Item
{
    width: 0
    height: 0
    visible: false
    property string title: i18n("Hardware and Sound")
    property var items: []

    Component
    {
        id: audioComponent
        AudioItem {}
    }

    Component
    {
        id: performanceComponent
        PerformanceItem {}
    }

    Component
    {
        id: inputDevicesComponent
        InputDevicesItem {}
    }

    Component
    {
        id: batteryComponent
        BatteryItem {}
    }

    Component.onCompleted:
    {
        items = [
            audioComponent.createObject(this),
            performanceComponent.createObject(this),
            inputDevicesComponent.createObject(this),
            batteryComponent.createObject(this)
        ]
    }
}
