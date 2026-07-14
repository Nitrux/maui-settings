import QtQuick

Item
{
    width: 0
    height: 0
    visible: false
    property string title: i18n("Connectivity")
    property var items: []

    Component
    {
        id: bluetoothComponent
        BluetoothItem {}
    }

    Component
    {
        id: networkComponent
        NetworkItem {}
    }

    Component.onCompleted:
    {
        items = [
            bluetoothComponent.createObject(this),
            networkComponent.createObject(this)
        ]
    }
}
