import QtQuick

Item
{
    width: 0
    height: 0
    visible: false
    property string title: i18n("Applications")
    property var items: []

    Component
    {
        id: defaultsComponent
        DefaultsItem {}
    }

    Component
    {
        id: cacheComponent
        CacheItem {}
    }

    Component.onCompleted:
    {
        items = [
            defaultsComponent.createObject(this),
            cacheComponent.createObject(this)
        ]
    }
}
