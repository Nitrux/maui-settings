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
        id: defaultAppsComponent
        DefaultAppsItem {}
    }

    Component.onCompleted:
    {
        items = [
            defaultsComponent.createObject(this),
            defaultAppsComponent.createObject(this)
        ]
    }
}
