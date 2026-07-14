import QtQuick

Item
{
    width: 0
    height: 0
    visible: false
    property string title: i18n("Appearance")
    property var items: []

    Component
    {
        id: backgroundComponent
        BackgroundItem {}
    }

    Component
    {
        id: themeComponent
        ThemeItem {}
    }

    Component.onCompleted:
    {
        items = [
            backgroundComponent.createObject(this),
            themeComponent.createObject(this)
        ]
    }
}
