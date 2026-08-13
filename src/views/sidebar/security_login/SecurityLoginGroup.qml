import QtQuick

Item
{
    width: 0
    height: 0
    visible: false
    property string title: i18n("Security and Login")
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

    Component
    {
        id: autostartComponent
        AutostartItem {}
    }

    Component
    {
        id: environmentVariablesComponent
        EnvironmentVariablesItem {}
    }

    Component.onCompleted:
    {
        items = [
            greeterComponent.createObject(this),
            lockScreenComponent.createObject(this),
            autostartComponent.createObject(this),
            environmentVariablesComponent.createObject(this)
        ]
    }
}
