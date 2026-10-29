import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.mauikit.filebrowsing as FB

Maui.ScrollColumn
{
    id: root
    readonly property var controller: (typeof bluetoothController !== "undefined" && bluetoothController) ? bluetoothController : null
    property string selectedDevicePath: ""
    property string selectedDeviceName: ""
    property bool selectedDeviceTrusted: false
    readonly property bool adapterNameSaveAvailable: root.controller
                                                    && _adapterNameField.text.trim().length > 0
                                                    && _adapterNameField.text.trim() !== root.controller.adapterName

    function saveAdapterName()
    {
        if (!adapterNameSaveAvailable || !root.controller)
            return false

        root.controller.setAdapterName(_adapterNameField.text)
        return true
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    onVisibleChanged:
    {
        if (!visible)
        {
            _pairingDialog.close()
            _deviceEditor.close()
            _errorToast.close()
        }
    }

    Maui.Popup
    {
        id: _errorToast
        parent: Overlay.overlay
        anchors.centerIn: null
        x: parent ? Math.round((parent.width - width) / 2) : 0
        y: parent ? parent.height - height - Maui.Style.contentMargins : 0
        width: parent
               ? Math.min(parent.width - Maui.Style.contentMargins * 2, Maui.Style.units.gridUnit * 24)
               : Maui.Style.units.gridUnit * 24
        height: _errorToastItem.implicitHeight + Maui.Style.contentMargins * 2
        modal: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: Maui.Style.contentMargins

        property string message: ""

        function showError(message)
        {
            _errorToast.message = message
            _errorToast.open()
            _errorToastTimer.restart()
        }

        Maui.SectionItem
        {
            id: _errorToastItem
            anchors.fill: parent
            flat: true
            template.iconSource: "dialog-error"
            label1.text: i18n("Bluetooth error")
            label1.elide: Text.ElideRight
            label2.text: _errorToast.message
            label2.wrapMode: Text.Wrap
        }

        Timer
        {
            id: _errorToastTimer
            interval: 5000
            onTriggered: _errorToast.close()
        }
    }

    FB.FileListingDialog
    {
        id: _pairingDialog
        property string requestId: ""

        title: root.controller && root.controller.pairingServiceUuid.length > 0
               ? i18n("Bluetooth service request")
               : i18n("Bluetooth pairing request")
        persistent: true

        template.visible: true
        template.iconVisible: false
        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.big

            Label
            {
                Layout.fillWidth: true
                text: root.controller
                      ? root.controller.pairingServiceUuid.length > 0
                        ? i18n("Allow %1 to use Bluetooth service %2?",
                               root.controller.pairingDeviceName,
                               root.controller.pairingServiceUuid)
                        : root.controller.pairingCode.length > 0
                        ? root.controller.pairingConfirmationRequired
                          ? i18n("Pair with %1? Confirm that this passkey is shown on the other device.", root.controller.pairingDeviceName)
                          : i18n("Enter this passkey on %1 to complete pairing.", root.controller.pairingDeviceName)
                        : i18n("Allow %1 to pair with this computer?", root.controller.pairingDeviceName)
                      : ""
                textFormat: Text.PlainText
                wrapMode: Text.WordWrap
            }

            Label
            {
                Layout.fillWidth: true
                visible: root.controller && root.controller.pairingDeviceAddress.length > 0
                text: root.controller ? i18n("Device address: %1", root.controller.pairingDeviceAddress) : ""
                textFormat: Text.PlainText
                opacity: 0.7
                wrapMode: Text.WrapAnywhere
            }

            Rectangle
            {
                Layout.fillWidth: true
                visible: root.controller && root.controller.pairingCode.length > 0
                implicitHeight: _pairingCodeLabel.implicitHeight + Maui.Style.contentMargins * 2
                radius: Maui.Style.radiusV
                color: Maui.Theme.alternateBackgroundColor

                Label
                {
                    id: _pairingCodeLabel
                    anchors.centerIn: parent
                    text: root.controller ? root.controller.pairingCode : ""
                    font.pointSize: Maui.Style.fontSizes.enormous
                    font.weight: Font.Medium
                    font.letterSpacing: 2
                }
            }
        }

        actions: [
            Action
            {
                text: root.controller && root.controller.pairingConfirmationRequired ? i18n("Deny") : i18n("Close")
                onTriggered:
                {
                    if (root.controller)
                    {
                        if (_pairingDialog.requestId.length > 0)
                            root.controller.respondToPairingPrompt(_pairingDialog.requestId, false)
                        else
                            root.controller.dismissPairingPrompt()
                    }
                }
            },
            Action
            {
                text: root.controller && root.controller.pairingServiceUuid.length > 0
                      ? i18n("Allow")
                      : root.controller && root.controller.pairingConfirmationRequired
                        ? i18n("Pair")
                        : i18n("Done")
                onTriggered:
                {
                    if (root.controller)
                    {
                        if (_pairingDialog.requestId.length > 0)
                            root.controller.respondToPairingPrompt(_pairingDialog.requestId, true)
                        else
                            root.controller.dismissPairingPrompt()
                    }
                }
            }
        ]

        onClosed:
        {
            if (root.controller && root.controller.pairingPromptActive)
            {
                if (_pairingDialog.requestId.length > 0)
                    root.controller.respondToPairingPrompt(_pairingDialog.requestId, false)
                else
                    root.controller.dismissPairingPrompt()
            }
        }
    }

    FB.FileListingDialog
    {
        id: _deviceEditor
        title: i18n("Edit Bluetooth Device")
        persistent: true

        template.visible: true
        template.iconVisible: false
        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Device name")
                label1.elide: Text.ElideRight
                label2.text: i18n("Use a local name that is easier to recognize.")
                label2.wrapMode: Text.Wrap
                template.content: TextField
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    id: _deviceNameField
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                    placeholderText: i18n("Device name")
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Trusted device")
                label1.elide: Text.ElideRight
                label2.text: i18n("Allow future connections without another authorization prompt.")
                label2.wrapMode: Text.Wrap
                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }
                    id: _trustedSwitch
                }
            }
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: _deviceEditor.close()
            },
            Action
            {
                text: i18n("Save")
                enabled: _deviceNameField.text.trim().length > 0
                onTriggered:
                {
                    if (root.controller)
                        root.controller.updateDevice(root.selectedDevicePath, _deviceNameField.text, _trustedSwitch.checked)
                    _deviceEditor.close()
                }
            }
        ]

        onOpened:
        {
            _deviceNameField.text = root.selectedDeviceName
            _trustedSwitch.checked = root.selectedDeviceTrusted
            _deviceNameField.forceActiveFocus()
        }
    }

    Connections
    {
        target: root.controller
        function onPairingPromptChanged()
        {
            if (root.controller.pairingPromptActive)
            {
                _pairingDialog.requestId = root.controller.pairingRequestId
                _pairingDialog.open()
            }
            else
            {
                _pairingDialog.close()
                _pairingDialog.requestId = ""
            }
        }

        function onErrorMessageChanged()
        {
            if (!root.controller || root.controller.errorMessage.length === 0)
                return

            _errorToast.showError(root.controller.errorMessage)
            root.controller.clearError()
        }

        function onAdapterNameChanged()
        {
            if (root.controller)
                _adapterNameField.text = root.controller.adapterName
        }
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Bluetooth")
        text2: i18n("Manage adapters, discover devices, and control paired connections.")
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _adapterLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _adapterLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Bluetooth Devices")
                text2: root.controller && root.controller.available
                       ? i18n("Ready to connect and discover nearby Bluetooth devices.")
                       : i18n("No Bluetooth adapter or BlueZ service was found.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Device name")
                label1.elide: Text.ElideRight
                label2.text: i18n("Name shown to nearby Bluetooth devices.")
                label2.wrapMode: Text.Wrap
                enabled: root.controller && root.controller.available
                template.content: RowLayout
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    spacing: Maui.Style.space.small

                    TextField
                    {
                        id: _adapterNameField
                        Layout.fillWidth: true
                        placeholderText: i18n("Bluetooth device name")
                        Component.onCompleted: text = root.controller ? root.controller.adapterName : ""
                        onAccepted: root.saveAdapterName()
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable Bluetooth")
                enabled: root.controller && root.controller.available
                label1.elide: Text.ElideRight
                label2.text: i18n("Allow this computer to communicate with Bluetooth devices.")
                label2.wrapMode: Text.Wrap
                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }
                    checked: root.controller ? root.controller.powered : false
                    enabled: root.controller && root.controller.available
                    onToggled: if (root.controller) root.controller.powered = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Device discovery")
                enabled: root.controller && root.controller.powered
                label1.elide: Text.ElideRight
                label2.text: root.controller && root.controller.discovering
                             ? i18n("Searching for nearby Bluetooth devices…")
                             : i18n("Search for nearby Bluetooth devices.")
                label2.wrapMode: Text.Wrap
                template.content: Button
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (!wideParent || !responsiveSectionItem)
                            return

                        parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                    }

                    onResponsiveNarrowChanged: updateResponsiveParent()

                    Component.onCompleted:
                    {
                        const originalParent = parent
                        responsiveSectionItem = originalParent.parent.parent.parent
                        wideParent = originalParent
                        updateResponsiveParent()
                    }
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    text: root.controller && root.controller.discovering ? i18n("Stop") : i18n("Scan")
                    enabled: root.controller && root.controller.powered
                    onClicked: root.controller.setDiscoveryEnabled(!root.controller.discovering)
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _pairedDevicesLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _pairedDevicesLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Paired devices")
                text2: i18n("Connect to or manage devices paired with this computer.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                id: _pairedDevicesRepeater
                model: root.controller

                delegate: Maui.SectionItem
                {
                    required property string name
                    required property string address
                    required property string iconName
                    required property bool paired
                    required property bool connected
                    required property bool trusted
                    required property int signalStrength
                    required property int batteryPercentage
                    required property string devicePath

                    Layout.fillWidth: true
                    visible: paired
                    flat: true
                    label1.text: name
                    label1.elide: Text.ElideRight
                    label2.text: batteryPercentage >= 0
                                 ? (connected
                                    ? i18n("Connected · %1 · Battery %2%", address, batteryPercentage)
                                    : i18n("Paired · %1 · Battery %2%", address, batteryPercentage))
                                 : (connected ? i18n("Connected · %1", address) : i18n("Paired · %1", address))
                    label2.wrapMode: Text.Wrap
                    template.iconSource: iconName.length > 0 ? iconName : "preferences-system-bluetooth"
                    template.iconSizeHint: Maui.Style.iconSizes.small
                    template.content: RowLayout
                    {
                        property Item wideParent
                        property Item responsiveSectionItem
                        readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                        function updateResponsiveParent()
                        {
                            if (!wideParent || !responsiveSectionItem)
                                return

                            parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent
                        }

                        onResponsiveNarrowChanged: updateResponsiveParent()

                        Component.onCompleted:
                        {
                            const originalParent = parent
                            responsiveSectionItem = originalParent.parent.parent.parent
                            wideParent = originalParent
                            updateResponsiveParent()
                        }
                        Layout.fillWidth: responsiveNarrow
                        Layout.minimumWidth: responsiveNarrow ? 0 : -1
                        Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                        spacing: Maui.Style.space.small

                        Button
                        {
                            text: connected ? i18n("Disconnect") : i18n("Connect")
                            onClicked: connected
                                       ? root.controller.disconnectDevice(devicePath)
                                       : root.controller.connectDevice(devicePath)
                        }

                        ToolSeparator
                        {
                            topPadding: 10
                            bottomPadding: 10
                        }

                        ToolButton
                        {
                            icon.name: "configure"
                            onClicked:
                            {
                                root.selectedDevicePath = devicePath
                                root.selectedDeviceName = name
                                root.selectedDeviceTrusted = trusted
                                _deviceEditor.open()
                            }
                        }

                        ToolButton
                        {
                            icon.name: "edit-delete"
                            ToolTip.visible: hovered
                            ToolTip.text: i18n("Unpair")
                            onClicked: root.controller.unpairDevice(devicePath)
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: root.controller && root.controller.pairedDeviceCount === 0
                flat: true
                label1.text: i18n("No paired devices")
                label1.elide: Text.ElideRight
                label2.text: i18n("Pair with a nearby device to see it here.")
                label2.wrapMode: Text.Wrap
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _availableDevicesLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _availableDevicesLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Available devices")
                text2: root.controller && root.controller.discovering
                       ? i18n("Searching for nearby Bluetooth devices…")
                       : i18n("Nearby devices that haven’t been paired with this computer.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                id: _availableDevicesRepeater
                model: root.controller

                delegate: Maui.SectionItem
                {
                    required property string name
                    required property string address
                    required property string iconName
                    required property bool paired
                    required property bool connected
                    required property int signalStrength
                    required property string devicePath

                    Layout.fillWidth: true
                    visible: !paired
                    flat: true
                    label1.text: name
                    label1.elide: Text.ElideRight
                    label2.text: signalStrength !== 0
                                 ? i18n("Available · RSSI %1 dBm", signalStrength)
                                 : i18n("Available · %1", address)
                    label2.wrapMode: Text.Wrap
                    template.iconSource: iconName.length > 0 ? iconName : "preferences-system-bluetooth"
                    template.iconSizeHint: Maui.Style.iconSizes.small
                    template.content: Button
                    {
                        text: i18n("Pair")
                        onClicked: root.controller.pairDevice(devicePath)
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: root.controller && root.controller.powered && root.controller.availableDeviceCount === 0
                flat: true
                label1.text: i18n("No available devices")
                label1.elide: Text.ElideRight
                label2.text: root.controller && root.controller.discovering
                             ? i18n("No nearby devices were found yet.")
                             : i18n("Start discovery and make sure the other device is visible.")
                label2.wrapMode: Text.Wrap
            }
        }
    }
}
