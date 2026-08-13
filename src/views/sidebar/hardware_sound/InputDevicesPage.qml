import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var info: (typeof hyprlandInfo !== "undefined" && hyprlandInfo) ? hyprlandInfo : null
    readonly property int controlWidth: Maui.Style.units.gridUnit * 13
    readonly property int textFieldWidth: Maui.Style.units.gridUnit * 13
    readonly property int spinBoxWidth: Maui.Style.units.gridUnit * 7
    property int inputItemType: 0
    property int editingKeybindIndex: -1
    property int editingDeviceIndex: -1
    property int pendingKeybindIndex: -1
    property int pendingDeviceIndex: -1
    property string recordedKey: ""
    property string editingKeybindOptions: ""
    property string editingKeybindExpression: ""
    property string editingKeybindAction: ""
    property string editingKeybindCommand: ""
    property string editingDeviceName: ""
    property real editingDeviceSensitivity: 0.0
    property var inputItemTypeLabels: [i18n("Keybind"), i18n("Device")]
    property var followMouseLabels: [i18n("Disabled"), i18n("Hover"), i18n("Click"), i18n("Button")]
    property var followMouseValues: [0, 1, 2, 3]
    property var keyboardLayouts: ["us", "gb", "de", "fr", "es", "it", "pt", "br", "ru", "ua", "pl", "cz", "sk", "hu", "tr", "se", "no", "dk", "fi", "nl", "be", "ch", "at", "jp", "kr", "il", "ara", "latam"]

    function responsive(control)
    {
        control.responsiveSectionItem = control.parent.parent.parent
        control.wideParent = control.parent
        control.updateResponsiveParent()
    }

    function indexForValue(model, value)
    {
        for (let i = 0; i < model.length; ++i)
            if (model[i] === value) return i
        return 0
    }

    function reloadSettings() { if (info) info.reload() }
    function saveSettings() { return info ? info.save() : false }

    function clearInputEditor()
    {
        inputItemType = 0
        editingKeybindIndex = -1
        editingDeviceIndex = -1
        pendingKeybindIndex = -1
        pendingDeviceIndex = -1
        editingKeybindOptions = ""
        editingKeybindExpression = ""
        editingKeybindAction = ""
        editingKeybindCommand = ""
        editingDeviceName = ""
        editingDeviceSensitivity = 0.0
        recordedKey = ""
        _keyField.text = ""
        _commandField.text = ""
    }

    function addInputItem()
    {
        clearInputEditor()
        _deviceNameField.text = ""
        _deviceSensitivitySpinBox.value = 0
        _inputDialog.open()
    }

    function addKeybind() { addInputItem() }

    function editKeybind(index, binding)
    {
        inputItemType = 0
        editingKeybindIndex = index
        editingDeviceIndex = -1
        recordedKey = binding.key || ""
        editingKeybindOptions = binding.options || ""
        editingKeybindExpression = binding.keyExpression || ""
        editingKeybindAction = binding.action || ""
        editingKeybindCommand = binding.command || ""
        _keyField.text = recordedKey
        _commandField.text = binding.command || binding.action || ""
        _inputDialog.open()
    }

    function editDevice(index, device)
    {
        inputItemType = 1
        editingKeybindIndex = -1
        editingDeviceIndex = index
        editingDeviceName = device.name || ""
        editingDeviceSensitivity = Number(device.sensitivity || 0)
        _deviceNameField.text = editingDeviceName
        _deviceSensitivitySpinBox.value = Math.round(editingDeviceSensitivity * 10)
        _inputDialog.open()
    }

    function applyInputItem()
    {
        if (!info) return
        if (inputItemType === 1)
        {
            if (!_deviceNameField.text.trim().length) return
            const updatedDevice = editingDeviceIndex >= 0
                ? info.updateDevice(editingDeviceIndex, _deviceNameField.text, _deviceSensitivitySpinBox.value / 10)
                : info.addDevice(_deviceNameField.text, _deviceSensitivitySpinBox.value / 10)
            if (updatedDevice) _inputDialog.close()
            return
        }

        if (!_keyField.text.trim().length || !_commandField.text.trim().length) return
        const preservedKey = editingKeybindIndex >= 0 && _keyField.text === recordedKey ? editingKeybindExpression : ""
        const preservedAction = editingKeybindIndex >= 0 && _commandField.text === editingKeybindCommand ? editingKeybindAction : ""
        const updated = editingKeybindIndex >= 0
            ? info.updateKeybind(editingKeybindIndex, _keyField.text, _commandField.text, editingKeybindOptions, preservedKey, preservedAction)
            : info.addKeybind(_keyField.text, _commandField.text)
        if (updated) _inputDialog.close()
    }

    function removeKeybind(index) { removeInputItem(0, index) }

    function removeInputItem(type, index)
    {
        inputItemType = type
        pendingKeybindIndex = type === 0 ? index : -1
        pendingDeviceIndex = type === 1 ? index : -1
        _removeInputDialog.open()
    }

    function keyName(key, text)
    {
        if (key >= Qt.Key_A && key <= Qt.Key_Z) return String.fromCharCode(key)
        if (key >= Qt.Key_0 && key <= Qt.Key_9) return String.fromCharCode(key)
        switch (key)
        {
        case Qt.Key_Left: return "left"
        case Qt.Key_Right: return "right"
        case Qt.Key_Up: return "up"
        case Qt.Key_Down: return "down"
        case Qt.Key_PageUp: return "pageup"
        case Qt.Key_PageDown: return "pagedown"
        case Qt.Key_Home: return "home"
        case Qt.Key_End: return "end"
        case Qt.Key_Insert: return "insert"
        case Qt.Key_Delete: return "delete"
        case Qt.Key_Backspace: return "backspace"
        case Qt.Key_Tab: return "tab"
        case Qt.Key_Return: return "Return"
        case Qt.Key_Enter: return "Enter"
        case Qt.Key_Escape: return "Escape"
        case Qt.Key_Space: return "space"
        case Qt.Key_Print: return "Print"
        case Qt.Key_F1: return "F1"
        case Qt.Key_F2: return "F2"
        case Qt.Key_F3: return "F3"
        case Qt.Key_F4: return "F4"
        case Qt.Key_F5: return "F5"
        case Qt.Key_F6: return "F6"
        case Qt.Key_F7: return "F7"
        case Qt.Key_F8: return "F8"
        case Qt.Key_F9: return "F9"
        case Qt.Key_F10: return "F10"
        case Qt.Key_F11: return "F11"
        case Qt.Key_F12: return "F12"
        default: return text && text.length ? text.toUpperCase() : ""
        }
    }

    function captureKey(event)
    {
        if (event.isAutoRepeat) return
        if (event.key === Qt.Key_Control || event.key === Qt.Key_Shift || event.key === Qt.Key_Alt || event.key === Qt.Key_Meta) return
        let modifiers = []
        if (event.modifiers & Qt.MetaModifier) modifiers.push("SUPER")
        if (event.modifiers & Qt.ControlModifier) modifiers.push("CTRL")
        if (event.modifiers & Qt.AltModifier) modifiers.push("ALT")
        if (event.modifiers & Qt.ShiftModifier) modifiers.push("SHIFT")
        const name = keyName(event.key, event.text)
        if (!name.length) return
        recordedKey = modifiers.length ? modifiers.join(" + ") + " + " + name : name
        editingKeybindExpression = ""
        _keyField.text = recordedKey
        event.accepted = true
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Input Devices")
        text2: !root.info || !root.info.available ? i18n("The Hyprland Lua configuration is not available.") : i18n("Configure keyboard, pointer, touchpad, and compositor shortcuts.")
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
        implicitHeight: _keyboardLayout.implicitHeight + Maui.Style.contentMargins * 2
        ColumnLayout
        {
            id: _keyboardLayout
            anchors.fill: parent; anchors.margins: Maui.Style.contentMargins; spacing: Maui.Style.space.small
            Maui.SectionHeader { Layout.fillWidth: true; text1: i18n("Keyboard"); text2: i18n("Choose the keyboard layout and XKB options used by Hyprland."); label2.wrapMode: Text.Wrap }
            Maui.SectionItem
            {
                Layout.fillWidth: true; flat: true
                label1.text: i18n("Layout"); label1.elide: Text.ElideRight
                label2.text: i18n("Choose an XKB layout code; this is not a locale or UTF-8 value."); label2.wrapMode: Text.Wrap
                template.content: ComboBox
                {
                    property Item wideParent; property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent(); Component.onCompleted: root.responsive(this)
                    editable: false; model: root.keyboardLayouts
                    currentIndex: root.indexForValue(root.keyboardLayouts, root.info ? root.info.keyboardLayout : "us")
                    Layout.fillWidth: responsiveNarrow; Layout.minimumWidth: responsiveNarrow ? 0 : -1; Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.controlWidth; Layout.preferredWidth: root.controlWidth
                    onActivated: if (root.info) root.info.keyboardLayout = root.keyboardLayouts[currentIndex]
                }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true; flat: true
                label1.text: i18n("Variant"); label1.elide: Text.ElideRight; label2.text: i18n("Optional XKB variant, for example dvorak."); label2.wrapMode: Text.Wrap
                template.content: TextField { Layout.fillWidth: true; Layout.maximumWidth: root.textFieldWidth; Layout.preferredWidth: root.textFieldWidth; placeholderText: i18n("Default"); text: root.info ? root.info.keyboardVariant : ""; onEditingFinished: if (root.info) root.info.keyboardVariant = text }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true; flat: true
                label1.text: i18n("Options"); label1.elide: Text.ElideRight; label2.text: i18n("Comma-separated XKB options, if required."); label2.wrapMode: Text.Wrap
                template.content: TextField { Layout.fillWidth: true; Layout.maximumWidth: root.textFieldWidth; Layout.preferredWidth: root.textFieldWidth; placeholderText: i18n("grp:alt_shift_toggle"); text: root.info ? root.info.keyboardOptions : ""; onEditingFinished: if (root.info) root.info.keyboardOptions = text }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true; flat: true
                label1.text: i18n("Model"); label1.elide: Text.ElideRight; label2.text: i18n("Optional XKB keyboard model."); label2.wrapMode: Text.Wrap
                template.content: TextField { Layout.fillWidth: true; Layout.maximumWidth: root.textFieldWidth; Layout.preferredWidth: root.textFieldWidth; placeholderText: i18n("Default"); text: root.info ? root.info.keyboardModel : ""; onEditingFinished: if (root.info) root.info.keyboardModel = text }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true; flat: true
                label1.text: i18n("Rules"); label1.elide: Text.ElideRight; label2.text: i18n("Optional XKB ruleset name."); label2.wrapMode: Text.Wrap
                template.content: TextField { Layout.fillWidth: true; Layout.maximumWidth: root.textFieldWidth; Layout.preferredWidth: root.textFieldWidth; placeholderText: i18n("Default"); text: root.info ? root.info.keyboardRules : ""; onEditingFinished: if (root.info) root.info.keyboardRules = text }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _pointerLayout.implicitHeight + Maui.Style.contentMargins * 2
        ColumnLayout
        {
            id: _pointerLayout
            anchors.fill: parent; anchors.margins: Maui.Style.contentMargins; spacing: Maui.Style.space.small
            Maui.SectionHeader { Layout.fillWidth: true; text1: i18n("Pointer and Touchpad"); text2: i18n("Control pointer focus behavior, sensitivity, and touchpad scrolling."); label2.wrapMode: Text.Wrap }
            Maui.SectionItem
            {
                Layout.fillWidth: true; flat: true
                label1.text: i18n("Focus follows pointer"); label1.elide: Text.ElideRight; label2.text: i18n("Choose when pointer movement changes the focused window."); label2.wrapMode: Text.Wrap
                template.content: ComboBox
                {
                    property Item wideParent; property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent(); Component.onCompleted: root.responsive(this)
                    model: root.followMouseLabels; currentIndex: root.info ? root.indexForValue(root.followMouseValues, root.info.followMouse) : 1
                    Layout.fillWidth: responsiveNarrow; Layout.minimumWidth: responsiveNarrow ? 0 : -1; Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.controlWidth; Layout.preferredWidth: root.controlWidth
                    onActivated: if (root.info) root.info.followMouse = root.followMouseValues[currentIndex]
                }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true; flat: true
                label1.text: i18n("Pointer sensitivity"); label1.elide: Text.ElideRight; label2.text: i18n("Adjust pointer sensitivity from -1.0 to 1.0."); label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    property Item wideParent; property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent(); Component.onCompleted: root.responsive(this)
                    from: -10; to: 10; stepSize: 1; value: root.info ? Math.round(root.info.pointerSensitivity * 10) : 0
                    textFromValue: function(value) { return (value / 10).toFixed(1) }; valueFromText: function(text) { return Math.round(Number(text) * 10) }
                    Layout.fillWidth: responsiveNarrow; Layout.minimumWidth: responsiveNarrow ? 0 : -1; Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.spinBoxWidth; Layout.preferredWidth: root.spinBoxWidth
                    onValueModified: if (root.info) root.info.pointerSensitivity = value / 10
                }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true; flat: true
                label1.text: i18n("Natural scrolling"); label1.elide: Text.ElideRight; label2.text: i18n("Reverse the touchpad scrolling direction."); label2.wrapMode: Text.Wrap
                template.content: Switch
                {
                    property Item wideParent; property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent(); Component.onCompleted: root.responsive(this)
                    checked: root.info ? root.info.naturalScroll : false; Layout.fillWidth: responsiveNarrow; Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : implicitWidth
                    onToggled: if (root.info) root.info.naturalScroll = checked
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _devicesLayout.implicitHeight + Maui.Style.contentMargins * 2
        ColumnLayout
        {
            id: _devicesLayout
            anchors.fill: parent; anchors.margins: Maui.Style.contentMargins; spacing: Maui.Style.space.small
            Maui.SectionHeader { Layout.fillWidth: true; text1: i18n("Devices"); text2: i18n("Configure sensitivity for specific input devices."); label2.wrapMode: Text.Wrap }
            Repeater
            {
                model: root.info ? root.info.devices : []
                delegate: Maui.SectionItem
                {
                    required property var modelData; required property int index
                    Layout.fillWidth: true; flat: true
                    label1.text: modelData.name || i18n("Unnamed device"); label1.elide: Text.ElideRight
                    label2.text: i18n("Sensitivity: %1", Number(modelData.sensitivity).toFixed(2)); label2.elide: Text.ElideRight
                    template.content: RowLayout
                    {
                        spacing: Maui.Style.space.tiny
                        ToolButton { icon.name: "document-edit"; display: ToolButton.IconOnly; ToolTip.visible: hovered; ToolTip.text: i18n("Edit device"); onClicked: root.editDevice(index, modelData) }
                        ToolButton { icon.name: "edit-delete"; display: ToolButton.IconOnly; ToolTip.visible: hovered; ToolTip.text: i18n("Remove device"); onClicked: root.removeInputItem(1, index) }
                    }
                }
            }
            Maui.SectionItem { Layout.fillWidth: true; flat: true; visible: !root.info || root.info.devices.length === 0; label1.text: i18n("No device overrides"); label2.text: i18n("Add a device with the + button in the header."); label2.wrapMode: Text.Wrap }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _keybindLayout.implicitHeight + Maui.Style.contentMargins * 2
        ColumnLayout
        {
            id: _keybindLayout
            anchors.fill: parent; anchors.margins: Maui.Style.contentMargins; spacing: Maui.Style.space.small
            Maui.SectionHeader { Layout.fillWidth: true; text1: i18n("Keybinds"); text2: i18n("Manage the keyboard shortcuts used by the Hyprland Lua configuration."); label2.wrapMode: Text.Wrap }
            Repeater
            {
                model: root.info ? root.info.keybinds : []
                delegate: Maui.SectionItem
                {
                    required property var modelData; required property int index
                    Layout.fillWidth: true; flat: true
                    label1.text: modelData.key || i18n("Unnamed keybind"); label1.elide: Text.ElideRight
                    label2.text: modelData.command || modelData.action || i18n("No action"); label2.elide: Text.ElideRight; label2.wrapMode: Text.NoWrap
                    template.content: RowLayout
                    {
                        spacing: Maui.Style.space.tiny
                        ToolButton { icon.name: "document-edit"; display: ToolButton.IconOnly; ToolTip.visible: hovered; ToolTip.text: i18n("Edit keybind"); onClicked: root.editKeybind(index, modelData) }
                        ToolButton { icon.name: "edit-delete"; display: ToolButton.IconOnly; ToolTip.visible: hovered; ToolTip.text: i18n("Remove keybind"); onClicked: root.removeKeybind(index) }
                    }
                }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true; flat: true; visible: !root.info || root.info.keybinds.length === 0
                label1.text: i18n("No keybinds"); label2.text: i18n("Add a shortcut with the + button in the header."); label2.wrapMode: Text.Wrap
            }
        }
    }

    Maui.SettingsDialog
    {
        id: _removeInputDialog
        title: root.inputItemType === 1 ? i18n("Remove Device") : i18n("Remove Keybind")
        persistent: true
        Label { Layout.fillWidth: true; text: root.inputItemType === 1 ? i18n("Remove this device override?") : i18n("Remove this keybind?"); wrapMode: Text.WordWrap }
        actions: [
            Action { text: i18n("Cancel"); onTriggered: _removeInputDialog.close() },
            Action
            {
                text: i18n("Remove")
                Maui.Controls.status: Maui.Controls.Negative
                onTriggered:
                {
                    if (root.info)
                    {
                        if (root.inputItemType === 1) root.info.removeDevice(root.pendingDeviceIndex)
                        else root.info.removeKeybind(root.pendingKeybindIndex)
                    }
                    _removeInputDialog.close()
                }
            }
        ]
        onClosed: { root.pendingKeybindIndex = -1; root.pendingDeviceIndex = -1 }
    }

    Maui.SettingsDialog
    {
        id: _inputDialog
        title: root.inputItemType === 1
            ? (root.editingDeviceIndex >= 0 ? i18n("Modify Device") : i18n("Add Device"))
            : (root.editingKeybindIndex >= 0 ? i18n("Modify Keybind") : i18n("Add Keybind"))
        persistent: true
        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Input type")
                label2.text: i18n("Choose whether to add a keybind or a device override.")
                ComboBox
                {
                    id: _inputTypeCombo
                    Layout.fillWidth: true
                    model: root.inputItemTypeLabels
                    currentIndex: root.inputItemType
                    enabled: root.editingKeybindIndex < 0 && root.editingDeviceIndex < 0
                    onActivated: root.inputItemType = currentIndex
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                visible: root.inputItemType === 0
                label1.text: i18n("Key combination")
                label2.text: i18n("Click the recorder, press the combination, and release the keys.")
                Button
                {
                    id: _recordKeyButton
                    Layout.fillWidth: true
                    text: root.recordedKey.length ? root.recordedKey : i18n("Click to record")
                    focusPolicy: Qt.StrongFocus
                    onClicked: { forceActiveFocus(); root.recordedKey = _keyField.text }
                    Keys.onPressed: function(event) { root.captureKey(event) }
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                visible: root.inputItemType === 0
                label1.text: i18n("Command")
                label2.text: i18n("Enter the command or dispatcher run by this shortcut.")
                Maui.TextField { id: _commandField; Layout.fillWidth: true; Layout.maximumWidth: root.textFieldWidth; Layout.preferredWidth: root.textFieldWidth; placeholderText: i18n("nudge-osd --volume-up"); onAccepted: root.applyInputItem() }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                visible: root.inputItemType === 1
                label1.text: i18n("Device name")
                label2.text: i18n("Use the exact name reported by Hyprland for this device.")
                Maui.TextField { id: _deviceNameField; Layout.fillWidth: true; Layout.maximumWidth: root.textFieldWidth; Layout.preferredWidth: root.textFieldWidth; placeholderText: i18n("epic-mouse-v1") }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                visible: root.inputItemType === 1
                label1.text: i18n("Sensitivity")
                label2.text: i18n("Set device sensitivity from -1.0 to 1.0.")
                SpinBox
                {
                    id: _deviceSensitivitySpinBox
                    from: -10; to: 10; stepSize: 1
                    textFromValue: function(value) { return (value / 10).toFixed(1) }
                    valueFromText: function(text) { return Math.round(Number(text) * 10) }
                    value: Math.round(root.editingDeviceSensitivity * 10)
                    Layout.maximumWidth: root.spinBoxWidth
                    Layout.preferredWidth: root.spinBoxWidth
                }
            }

            TextField { id: _keyField; visible: false; text: root.recordedKey }
        }
        actions: [
            Action { text: i18n("Cancel"); onTriggered: _inputDialog.close() },
            Action
            {
                text: root.inputItemType === 1 ? (root.editingDeviceIndex >= 0 ? i18n("Apply") : i18n("Save")) : (root.editingKeybindIndex >= 0 ? i18n("Apply") : i18n("Save"))
                enabled: root.info && (root.inputItemType === 1 ? _deviceNameField.text.trim().length > 0 : _keyField.text.trim().length > 0 && _commandField.text.trim().length > 0)
                onTriggered: root.applyInputItem()
            }
        ]
        onOpened:
        {
            _inputTypeCombo.currentIndex = root.inputItemType
            if (root.inputItemType === 1) _deviceNameField.forceActiveFocus()
            else _recordKeyButton.forceActiveFocus()
        }
        onClosed: root.clearInputEditor()
    }
}
