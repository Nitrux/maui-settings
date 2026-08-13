import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var controller: (typeof autostartController !== "undefined" && autostartController)
        ? autostartController : null
    readonly property int controlWidth: Maui.Style.units.gridUnit * 13
    property int editingIndex: -1

    function reloadSettings()
    {
        if (root.controller)
            root.controller.reload()
    }

    function openAddDialog()
    {
        root.editingIndex = -1
        _commandField.clear()
        _commandDialog.open()
        _commandField.forceActiveFocus()
    }

    function openEditDialog(index, command)
    {
        root.editingIndex = index
        _commandField.text = command
        _commandDialog.open()
        _commandField.selectAll()
        _commandField.forceActiveFocus()
    }

    function applyCommand()
    {
        if (!root.controller)
            return

        const command = _commandField.text.trim()
        const changed = root.editingIndex < 0
            ? root.controller.addCommand(command)
            : root.controller.updateCommand(root.editingIndex, command)

        if (changed)
            _commandDialog.close()
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SettingsDialog
    {
        id: _commandDialog
        title: root.editingIndex < 0 ? i18n("Add Autostart Command") : i18n("Edit Autostart Command")
        persistent: true

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Command")
                text2: i18n("Enter one command or command chain to run when Hyprland starts.")
                label2.wrapMode: Text.Wrap
            }

            TextField
            {
                id: _commandField
                Layout.fillWidth: true
                placeholderText: i18n("example-command --option")
                onAccepted: root.applyCommand()
            }
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: _commandDialog.close()
            },
            Action
            {
                text: root.editingIndex < 0 ? i18n("Add") : i18n("Apply")
                enabled: root.controller && _commandField.text.trim().length > 0
                onTriggered: root.applyCommand()
            }
        ]

        onClosed:
        {
            root.editingIndex = -1
            _commandField.clear()
        }
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Autostart")
        text2: !root.controller || !root.controller.available
            ? i18n("The Hyprland Lua configuration is not available.")
            : i18n("Manage autostart commands in Hyprland.")
        label2.wrapMode: Text.Wrap
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.controller ? root.controller.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _commandsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _commandsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Startup commands")
                text2: i18n("Configure the commands used during session startup.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                model: root.controller ? root.controller.commands : []

                delegate: Maui.SectionItem
                {
                    required property int index
                    required property var modelData

                    Layout.fillWidth: true
                    flat: true
                    label1.text: modelData
                    label1.elide: Text.ElideRight
                    label1.wrapMode: Text.NoWrap
                    label2.wrapMode: Text.Wrap

                    template.content: RowLayout
                    {
                        spacing: Maui.Style.space.tiny

                        ToolButton
                        {
                            icon.name: "document-edit"
                            display: ToolButton.IconOnly
                            ToolTip.visible: hovered
                            ToolTip.text: i18n("Edit command")
                            onClicked: root.openEditDialog(index, modelData)
                        }

                        ToolButton
                        {
                            icon.name: "edit-delete"
                            display: ToolButton.IconOnly
                            ToolTip.visible: hovered
                            ToolTip.text: i18n("Remove command")
                            onClicked:
                            {
                                if (root.controller)
                                    root.controller.removeCommand(index)
                            }
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: !root.controller || root.controller.commands.length === 0
                flat: true
                label1.text: i18n("No autostart commands")
                label2.text: i18n("Add a command to run it when the Hyprland session starts.")
                template.iconSource: "system-run"
            }
        }
    }

    Rectangle
    {
        visible: root.controller && root.controller.errorMessage.length > 0
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.negativeTextColor
        border.width: 1
        implicitHeight: _errorLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _errorLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Could Not Update Autostart")
                text2: root.controller ? root.controller.errorMessage : ""
                template.iconSource: "dialog-error"
                label2.wrapMode: Text.Wrap
            }

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: i18n("Dismiss")
                onClicked: root.controller.clearError()
            }
        }
    }

    Component.onCompleted: root.reloadSettings()
}
