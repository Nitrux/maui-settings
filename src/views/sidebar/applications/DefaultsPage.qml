import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.mauikit.filebrowsing as FB

Maui.ScrollColumn
{
    id: root
    readonly property var controller: (typeof mimeController !== "undefined" && mimeController) ? mimeController : null
    property string pendingRemoval: ""
    property bool editing: false

    function reloadSettings()
    {
        if (root.controller)
            root.controller.reload()
    }

    function editAssociation(mimeType, application)
    {
        root.editing = true
        _mimeTypeField.text = mimeType
        _applicationField.text = application
        _mimeTypeField.enabled = false
        _applicationField.forceActiveFocus()
    }

    function clearEditor()
    {
        root.editing = false
        _mimeTypeField.clear()
        _applicationField.clear()
        _mimeTypeField.enabled = true
    }

    function applyAssociation()
    {
        if (!root.controller)
            return

        if (root.controller.setAssociation(_mimeTypeField.text, _applicationField.text))
            root.clearEditor()
    }

    function pickApplication()
    {
        _applicationDialog.currentPath = FB.FM.homePath()
        _applicationDialog.browser.settings.viewType = FB.FMList.LIST_VIEW
        _applicationDialog.callback = (paths) =>
        {
            if (paths && paths.length)
                _applicationField.text = paths[0]
        }
        _applicationDialog.open()
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    FB.FileDialog
    {
        id: _applicationDialog
        singleSelection: true
        searchBar: true
        mode: FB.FileDialog.Modes.Open
        currentPath: FB.FM.homePath()
    }

    Maui.PopupPage
    {
        Maui.Controls.flat: true
        id: _removeDialog
        title: i18n("Remove Association")
        persistent: true
        maxWidth: Maui.Style.units.gridUnit * 24

        Label
        {
            Layout.fillWidth: true
            text: i18n("Remove the user default for %1?", root.pendingRemoval)
            wrapMode: Text.WordWrap
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: _removeDialog.close()
            },
            Action
            {
                text: i18n("Remove")
                Maui.Controls.status: Maui.Controls.Negative
                onTriggered:
                {
                    if (root.controller)
                        root.controller.removeAssociation(root.pendingRemoval)
                    _removeDialog.close()
                }
            }
        ]

        onClosed: root.pendingRemoval = ""
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("MIME Defaults")
        text2: i18n("Manage the applications used to open MIME types and links.")
    }

    Maui.SectionGroup
    {
        title: root.editing ? i18n("Modify Association") : i18n("Add Association")
        description: i18n("Use a desktop file ID or choose an executable.")

        background: Rectangle
        {
            color: Maui.Theme.alternateBackgroundColor
            radius: Maui.Style.radiusV
            border.color: Maui.Theme.backgroundColor
            border.width: 1
        }

        Maui.FlexSectionItem
        {
            Layout.fillWidth: true
            flat: true
            label1.text: i18n("MIME Type")
            label2.text: i18n("For example: text/plain or x-scheme-handler/https.")

            TextField
            {
                id: _mimeTypeField
                Layout.fillWidth: true
                Layout.minimumWidth: Maui.Style.units.gridUnit * 10
                Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                Layout.maximumWidth: Maui.Style.units.gridUnit * 18
                placeholderText: i18n("text/plain")
                onAccepted: _applicationField.forceActiveFocus()
            }
        }

        Maui.FlexSectionItem
        {
            Layout.fillWidth: true
            flat: true
            label1.text: i18n("Desktop File or Executable")
            label2.text: i18n("Enter a desktop file ID or select an executable.")

            RowLayout
            {
                Layout.fillWidth: true
                Layout.minimumWidth: Maui.Style.units.gridUnit * 12
                Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                Layout.maximumWidth: Maui.Style.units.gridUnit * 18

                TextField
                {
                    id: _applicationField
                    Layout.fillWidth: true
                    Layout.minimumWidth: 0
                    placeholderText: i18n("org.example.Application.desktop")
                    onAccepted: root.applyAssociation()
                }

                Button
                {
                    text: i18n("Choose")
                    onClicked: root.pickApplication()
                }
            }
        }

        Maui.FlexSectionItem
        {
            Layout.fillWidth: true
            flat: true
            label1.text: root.editing ? i18n("Apply Changes") : i18n("Add Association")
            label2.text: root.editing
                         ? i18n("Replace the application assigned to this MIME type.")
                         : i18n("Save to mimeapps.list.")

            RowLayout
            {
                Layout.fillWidth: true
                spacing: Maui.Style.space.small

                Item
                {
                    Layout.fillWidth: true
                }

                Button
                {
                    visible: root.editing
                    text: i18n("Cancel")
                    onClicked: root.clearEditor()
                }

                Button
                {
                    text: root.editing ? i18n("Apply") : i18n("Add")
                    enabled: root.controller && _mimeTypeField.text.trim().length > 0 && _applicationField.text.trim().length > 0
                    highlighted: true
                    onClicked: root.applyAssociation()
                }
            }
        }
    }

    Maui.SectionGroup
    {
        title: i18n("MIME Type Associations")
        description: i18n("User-defined handlers.")

        background: Rectangle
        {
            color: Maui.Theme.alternateBackgroundColor
            radius: Maui.Style.radiusV
            border.color: Maui.Theme.backgroundColor
            border.width: 1
        }

        Repeater
        {
            model: root.controller ? root.controller.associations : []

            delegate: Maui.SectionItem
            {
                required property var modelData

                Layout.fillWidth: true
                flat: true
                label1.text: modelData.mimeType
                label1.elide: Text.ElideRight
                label1.wrapMode: Text.NoWrap
                label2.text: modelData.application
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
                        ToolTip.text: i18n("Edit association")
                        onClicked: root.editAssociation(modelData.mimeType, modelData.application)
                    }

                    ToolButton
                    {
                        icon.name: "edit-delete"
                        display: ToolButton.IconOnly
                        ToolTip.visible: hovered
                        ToolTip.text: i18n("Remove association")
                        onClicked:
                        {
                            root.pendingRemoval = modelData.mimeType
                            _removeDialog.open()
                        }
                    }
                }
            }
        }

        Maui.SectionItem
        {
            Layout.fillWidth: true
            visible: !root.controller || root.controller.associations.length === 0
            flat: true
            label1.text: i18n("No user-defined defaults")
            label2.text: i18n("Add an association below to create the Default Applications group.")
            template.iconSource: "documentinfo"
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
                text1: i18n("Could Not Update Defaults")
                text2: root.controller ? root.controller.errorMessage : ""
                template.iconSource: "dialog-error"
            }

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: i18n("Dismiss")
                onClicked: root.controller.clearError()
            }
        }
    }

    Component.onCompleted: reloadSettings()
}
