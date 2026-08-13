import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var controller: (typeof environmentController !== "undefined" && environmentController)
        ? environmentController : null
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
        _variableField.clear()
        _valueField.clear()
        _variableDialog.open()
        _variableField.forceActiveFocus()
    }

    function openEditDialog(index, variable, value)
    {
        root.editingIndex = index
        _variableField.text = variable
        _valueField.text = value
        _variableDialog.open()
        _variableField.selectAll()
        _variableField.forceActiveFocus()
    }

    function applyVariable()
    {
        if (!root.controller)
            return

        const variable = _variableField.text.trim()
        const value = _valueField.text.trim()
        const changed = root.editingIndex < 0
            ? root.controller.addVariable(variable, value)
            : root.controller.updateVariable(root.editingIndex, variable, value)

        if (changed)
            _variableDialog.close()
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SettingsDialog
    {
        id: _variableDialog
        title: root.editingIndex < 0
            ? i18n("Add Environment Variable")
            : i18n("Edit Environment Variable")
        persistent: true

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Variable")
                label2.text: i18n("Use a valid shell variable name, such as GTK_USE_PORTAL.")
                label2.wrapMode: Text.Wrap

                TextField
                {
                    id: _variableField
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    Layout.preferredWidth: root.controlWidth
                    Layout.maximumWidth: root.controlWidth
                    placeholderText: i18n("VARIABLE_NAME")
                    onAccepted: _valueField.forceActiveFocus()
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Value")
                label2.text: i18n("The value exported to applications in the Hyprland session.")
                label2.wrapMode: Text.Wrap

                TextField
                {
                    id: _valueField
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    Layout.preferredWidth: root.controlWidth
                    Layout.maximumWidth: root.controlWidth
                    placeholderText: i18n("value")
                    onAccepted: root.applyVariable()
                }
            }
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: _variableDialog.close()
            },
            Action
            {
                text: root.editingIndex < 0 ? i18n("Add") : i18n("Apply")
                enabled: root.controller && _variableField.text.trim().length > 0
                onTriggered: root.applyVariable()
            }
        ]

        onClosed:
        {
            root.editingIndex = -1
            _variableField.clear()
            _valueField.clear()
        }
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Environment Variables")
        text2: !root.controller || !root.controller.available
            ? i18n("The Hyprland Lua configuration is not available.")
            : i18n("Manage variables exported to applications in the Hyprland session.")
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
        implicitHeight: _variablesLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _variablesLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Hyprland Evironment")
                text2: i18n("Manage variables as separate entries.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                model: root.controller ? root.controller.variables : []

                delegate: Maui.SectionItem
                {
                    required property int index
                    required property var modelData

                    Layout.fillWidth: true
                    flat: true
                    label1.text: modelData.variable
                    label1.elide: Text.ElideRight
                    label1.wrapMode: Text.NoWrap
                    label2.text: modelData.value
                    label2.elide: Text.ElideRight
                    label2.wrapMode: Text.NoWrap

                    template.content: RowLayout
                    {
                        spacing: Maui.Style.space.tiny

                        ToolButton
                        {
                            icon.name: "document-edit"
                            display: ToolButton.IconOnly
                            ToolTip.visible: hovered
                            ToolTip.text: i18n("Edit variable")
                            onClicked: root.openEditDialog(index, modelData.variable, modelData.value)
                        }

                        ToolButton
                        {
                            icon.name: "edit-delete"
                            display: ToolButton.IconOnly
                            ToolTip.visible: hovered
                            ToolTip.text: i18n("Remove variable")
                            onClicked:
                            {
                                if (root.controller)
                                    root.controller.removeVariable(index)
                            }
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: !root.controller || root.controller.variables.length === 0
                flat: true
                label1.text: i18n("No environment variables")
                label2.text: i18n("Add a variable to export it in the Hyprland session.")
                template.iconSource: "preferences-system"
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
                text1: i18n("Could Not Update Environment")
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
