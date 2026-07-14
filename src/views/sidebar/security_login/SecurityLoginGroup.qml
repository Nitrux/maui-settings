import QtQuick

Item
{
    width: 0
    height: 0
    visible: false
    property string title: i18n("Security & Login")
    property var items: []

    Component
    {
        id: greeterComponent
        GreeterItem {}
    }

    Component
    {
        id: lockScreenComponent
        LockScreenItem {}
    }

    Component.onCompleted:
    {
        items = [
            greeterComponent.createObject(this),
            lockScreenComponent.createObject(this)
        ]
    }
}
