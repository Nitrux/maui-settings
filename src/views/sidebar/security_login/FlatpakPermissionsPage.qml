import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root

    readonly property var controller: (typeof flatpakPermissionsController !== "undefined" && flatpakPermissionsController)
        ? flatpakPermissionsController : null
    readonly property int dialogControlWidth: Maui.Style.units.gridUnit * 8
    readonly property bool saveAvailable: root.controller && root.controller.isSaveNeeded
    readonly property bool resetAvailable: root.controller && !root.controller.isDefaults
    property var sectionCards: [
        {
            section: "application",
            headerText: i18n("Flatpak applications"),
            details: i18n("Select an installed Flatpak application to manage its sandbox permissions.")
        }
    ]

    readonly property var commonPermissionNames: [
        "network", "pulseaudio", "bluetooth", "cups", "dri", "x11", "wayland"
    ]
    readonly property var filesystemPermissionNames: ["home", "host", "host-etc"]

    readonly property var filesystemOptions: [
        i18n("Read-only"),
        i18n("Read/write"),
        i18n("Read/write and create"),
        i18n("OFF")
    ]
    readonly property var filesystemOptionValues: ["ro", "rw", "create", "off"]
    function reloadSettings()
    {
        if (root.controller)
            root.controller.reload()
    }

    function saveSettings()
    {
        return root.controller ? root.controller.save() : false
    }

    function resetSettings()
    {
        if (root.controller)
            root.controller.defaults()
    }

    function permissionIndex(permission)
    {
        if (!root.controller)
            return -1

        const permissions = root.controller.permissions
        for (let index = 0; index < permissions.length; ++index)
        {
            if (permissions[index].section === permission.section
                && permissions[index].name === permission.name)
                return index
        }
        return -1
    }

    function dialogPermissions(type)
    {
        if (!root.controller)
            return []

        const permissions = root.controller.permissions
        if (type === "simple")
            return permissions.filter((permission) => root.commonPermissionNames.indexOf(permission.name) >= 0)
        if (type === "filesystem")
            return permissions.filter((permission) => root.filesystemPermissionNames.indexOf(permission.name) >= 0)
        if (type === "environment")
            return permissions.filter((permission) => permission.type === "environment")
        return []
    }

    function filesystemOptionIndex(value)
    {
        for (let index = 0; index < root.filesystemOptions.length; ++index)
        {
            if (root.filesystemOptionValues[index] === value)
                return index
        }
        return 0
    }

    function setDialogPermission(permission, enabled)
    {
        const index = root.permissionIndex(permission)
        if (index >= 0 && root.controller)
            root.controller.setPermission(index, enabled)
    }

    function setDialogPermissionValue(permission, value)
    {
        const index = root.permissionIndex(permission)
        if (index >= 0 && root.controller)
            root.controller.setPermissionValue(index, value)
    }

    function openPermissionsDialog(appId)
    {
        if (!root.controller)
            return
        root.controller.selectApplication(appId)
        _permissionsDialog.open()
    }

    anchors.fill: parent
    spacing: Maui.Style.space.medium

    Maui.SettingsDialog
    {
        id: _permissionsDialog
        title: root.controller && root.controller.selectedAppName.length > 0
            ? root.controller.selectedAppName
            : i18n("Flatpak Permissions")
        persistent: true

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                visible: root.dialogPermissions("simple").length > 0
                Layout.fillWidth: true
                text1: i18n("Common access")
                text2: i18n("Control the permissions most applications use for networking, sound, devices, and display output.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                model: root.dialogPermissions("simple")

                delegate: Maui.FlexSectionItem
                {
                    required property var modelData

                    Layout.fillWidth: true
                    flat: true
                    label1.text: modelData.label
                    label2.text: modelData.explanation
                    label2.wrapMode: Text.Wrap

                    Switch
                    {
                        checked: modelData.enabled
                        enabled: modelData.canBeDisabled
                        Layout.alignment: Qt.AlignRight
                        onToggled: root.setDialogPermission(modelData, checked)
                    }
                }
            }

            Maui.SectionHeader
            {
                visible: root.dialogPermissions("filesystem").length > 0
                Layout.fillWidth: true
                text1: i18n("File access")
                text2: i18n("Choose how the application can access its home and system configuration files.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                model: root.dialogPermissions("filesystem")

                delegate: Maui.FlexSectionItem
                {
                    required property var modelData

                    Layout.fillWidth: true
                    flat: true
                    label1.text: modelData.label
                    label2.text: modelData.explanation
                    label2.wrapMode: Text.Wrap

                    ComboBox
                    {
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: root.dialogControlWidth
                        Layout.maximumWidth: root.dialogControlWidth
                        model: root.filesystemOptions
                        currentIndex: root.filesystemOptionIndex(modelData.value)
                        onActivated: root.setDialogPermissionValue(modelData, root.filesystemOptionValues[currentIndex])
                    }
                }
            }

            Maui.SectionHeader
            {
                visible: root.dialogPermissions("environment").length > 0
                Layout.fillWidth: true
                text1: i18n("Environment")
                text2: i18n("Adjust the environment values passed to the application.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                model: root.dialogPermissions("environment")

                delegate: Maui.FlexSectionItem
                {
                    required property var modelData

                    Layout.fillWidth: true
                    flat: true
                    label1.text: modelData.label
                    label2.text: modelData.explanation
                    label2.wrapMode: Text.Wrap

                    TextField
                    {
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: root.dialogControlWidth
                        Layout.maximumWidth: root.dialogControlWidth
                        text: modelData.value
                        onEditingFinished: root.setDialogPermissionValue(modelData, text)
                    }
                }
            }
        }

        actions: [
            Action
            {
                text: i18n("Done")
                onTriggered: _permissionsDialog.close()
            }
        ]
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Flatpak Permissions")
        text2: !root.controller || !root.controller.available
            ? i18n("Flatpak is not available on this system.")
            : root.controller.errorMessage.length > 0
                ? root.controller.errorMessage
                : root.controller.selectedAppId.length === 0
                    ? i18n("Install a Flatpak application to manage its sandbox permissions.")
                    : i18n("Manage the sandbox permissions granted to Flatpak applications.")
        label2.wrapMode: Text.Wrap
    }

    Repeater
    {
        model: root.sectionCards

        delegate: Rectangle
        {
            required property var modelData
            readonly property var cardData: modelData

            visible: cardData.section === "application"
                || (root.controller && root.controller.selectedAppId.length > 0)
            Layout.fillWidth: true
            color: Maui.Theme.alternateBackgroundColor
            radius: Maui.Style.radiusV
            border.color: Maui.Theme.backgroundColor
            border.width: 1
            implicitHeight: cardLayout.implicitHeight + Maui.Style.contentMargins * 2

            ColumnLayout
            {
                id: cardLayout
                anchors.fill: parent
                anchors.margins: Maui.Style.contentMargins
                spacing: Maui.Style.space.small

                Maui.SectionHeader
                {
                    Layout.fillWidth: true
                    text1: cardData.headerText
                    text2: cardData.details
                    label2.wrapMode: Text.Wrap
                }

                Repeater
                {
                    model: cardData.section === "application" && root.controller ? root.controller.applications : []

                    delegate: Maui.FlexSectionItem
                    {
                        required property var modelData

                        Layout.fillWidth: true
                        flat: true
                        iconSource: modelData.icon
                        iconSizeHint: Maui.Style.iconSizes.big
                        label1.text: modelData.name
                        label1.font.weight: Font.DemiBold
                        label1.elide: Text.ElideRight
                        label2.text: modelData.id
                        label2.elide: Text.ElideRight

                        ToolButton
                        {
                            icon.name: "configure"
                            display: ToolButton.IconOnly
                            ToolTip.visible: hovered
                            ToolTip.text: i18n("Manage permissions")
                            onClicked: root.openPermissionsDialog(modelData.id)
                        }

                        onClicked: root.openPermissionsDialog(modelData.id)
                    }
                }

                Maui.FlexSectionItem
                {
                    visible: cardData.section === "application" && (!root.controller || root.controller.applications.length === 0)
                    Layout.fillWidth: true
                    flat: true
                    label1.text: i18n("No Flatpak applications found")
                    label2.text: i18n("Install a Flatpak application before configuring its sandbox permissions.")
                    label2.wrapMode: Text.Wrap
                    iconSource: "application-x-flatpak"
                }

            }
        }
    }

    Component.onCompleted: root.reloadSettings()
}
