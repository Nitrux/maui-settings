import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var info: (typeof hyprlandInfo !== "undefined" && hyprlandInfo) ? hyprlandInfo : null
    readonly property int controlWidth: Maui.Style.units.gridUnit * 13

    property var applicationGroups: [
        {
            key: "terminal",
            title: i18n("Terminal"),
            description: i18n("Command used to open a terminal.")
        },
        {
            key: "fileManager",
            title: i18n("File manager"),
            description: i18n("Command used to browse files and folders.")
        },
        {
            key: "menu",
            title: i18n("Application menu"),
            description: i18n("Command used to open the application launcher.")
        },
        {
            key: "lockScreen",
            title: i18n("Lock screen"),
            description: i18n("Command used to lock the current session.")
        },
        {
            key: "webBrowser",
            title: i18n("Web browser"),
            description: i18n("Command used to open web links.")
        }
    ]

    function reloadSettings()
    {
        if (root.info)
            root.info.reload()
    }

    function saveSettings()
    {
        return root.info ? root.info.save() : false
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Default Applications")
        text2: !root.info || !root.info.available
            ? i18n("The Hyprland Lua configuration is not available.")
            : i18n("Choose the applications used by core desktop actions.")
        label2.wrapMode: Text.Wrap
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _applicationsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _applicationsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Desktop applications")
                text2: i18n("These commands are used by the Hyprland desktop configuration.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                model: root.applicationGroups

                delegate: Maui.SectionItem
                {
                    required property var modelData

                    Layout.fillWidth: true
                    flat: true
                    label1.text: modelData.title
                    label1.elide: Text.ElideRight
                    label2.text: modelData.description
                    label2.wrapMode: Text.Wrap

                    template.content: TextField
                    {
                        property Item wideParent
                        property Item responsiveSectionItem
                        readonly property bool responsiveNarrow: responsiveSectionItem
                            && (Maui.Handy.isMobile
                                || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                        function updateResponsiveParent()
                        {
                            if (!wideParent || !responsiveSectionItem)
                                return

                            const nextParent = responsiveNarrow
                                ? responsiveSectionItem.contentItem
                                : wideParent
                            if (nextParent && parent !== nextParent)
                                parent = nextParent
                        }

                        onResponsiveNarrowChanged: updateResponsiveParent()

                        Component.onCompleted:
                        {
                            const originalParent = parent
                            if (!originalParent || !originalParent.parent
                                || !originalParent.parent.parent
                                || !originalParent.parent.parent.parent)
                                return

                            responsiveSectionItem = originalParent.parent.parent.parent
                            wideParent = originalParent
                            updateResponsiveParent()
                        }

                        Layout.fillWidth: responsiveNarrow
                        Layout.minimumWidth: responsiveNarrow ? 0 : -1
                        Layout.maximumWidth: responsiveNarrow
                            ? Number.POSITIVE_INFINITY
                            : root.controlWidth
                        Layout.preferredWidth: root.controlWidth
                        placeholderText: i18n("Executable or command")
                        text: root.info ? root.info[modelData.key] : ""
                        onEditingFinished:
                        {
                            if (root.info)
                                root.info[modelData.key] = text.trim()
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: root.reloadSettings()
}
