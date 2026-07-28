import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var controller: (typeof networkController !== "undefined" && networkController) ? networkController : null
    property string selectedDevicePath: ""
    property string selectedAccessPointPath: ""
    property string selectedSsid: ""
    property bool selectedAutoConnect: false

    function connectSelectedNetwork()
    {
        if (!root.controller || _passwordField.text.length === 0)
            return

        root.controller.connectToNetwork(root.selectedDevicePath, root.selectedAccessPointPath, root.selectedSsid, _passwordField.text)
        _passwordDialog.close()
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    onVisibleChanged:
    {
        if (!visible)
        {
            _passwordDialog.close()
            _networkEditor.close()
        }
    }

    Maui.PopupPage
    {
        id: _passwordDialog
        title: i18n("Connect to %1", root.selectedSsid)
        persistent: true
        maxWidth: Maui.Style.units.gridUnit * 24

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.medium

            RowLayout
            {
                Layout.fillWidth: true
                spacing: Maui.Style.space.medium

                Maui.IconItem
                {
                    iconSource: "network-wireless-locked"
                    iconSizeHint: Maui.Style.iconSizes.huge
                }

                ColumnLayout
                {
                    Layout.fillWidth: true
                    spacing: Maui.Style.space.tiny

                    Label
                    {
                        Layout.fillWidth: true
                        text: i18n("Network password")
                        font.weight: Font.DemiBold
                        wrapMode: Text.WordWrap
                    }

                    Label
                    {
                        Layout.fillWidth: true
                        text: i18n("Enter the password for %1.", root.selectedSsid)
                        color: Maui.Theme.textColor
                        opacity: 0.7
                        wrapMode: Text.WordWrap
                    }
                }
            }

            Maui.PasswordField
            {
                id: _passwordField
                Layout.fillWidth: true
                placeholderText: i18n("Password")
                onAccepted: root.connectSelectedNetwork()
            }
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: _passwordDialog.close()
            },
            Action
            {
                id: _connectAction
                text: i18n("Connect")
                enabled: _passwordField.text.length > 0
                onTriggered: root.connectSelectedNetwork()
            }
        ]

        onOpened: _passwordField.forceActiveFocus()
        onClosed: _passwordField.clear()
    }

    Maui.PopupPage
    {
        id: _networkEditor
        title: i18n("Edit %1", root.selectedSsid)
        persistent: true
        maxWidth: Maui.Style.units.gridUnit * 24

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Connect automatically")
                label1.elide: Text.ElideRight
                label2.text: i18n("Join this network whenever it is available.")
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
                    id: _autoConnectSwitch
                    checked: root.selectedAutoConnect
                    onClicked:
                    {
                        root.selectedAutoConnect = checked
                        if (root.controller)
                            root.controller.updateSavedNetwork(root.selectedDevicePath, root.selectedSsid, checked)
                    }
                }
            }
        }

        actions: [
            Action
            {
                text: i18n("Forget")
                Maui.Controls.status: Maui.Controls.Negative
                onTriggered:
                {
                    if (root.controller)
                        root.controller.forgetNetwork(root.selectedDevicePath, root.selectedSsid)
                    _networkEditor.close()
                }
            }
        ]
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Network")
        text2: i18n("Manage wired and wireless network connections.")
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _wiredLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _wiredLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Wired Connectivity")
            description: root.controller && root.controller.wiredConnections.length > 0
                       ? i18n("Ethernet interfaces and saved wired connections.")
                       : i18n("No Ethernet interface was found.")
            template.label2.wrapMode: Text.Wrap

            Repeater
            {
                model: root.controller ? root.controller.wiredConnections : []

                delegate: Maui.SectionItem
                {
                    required property var modelData

                    Layout.fillWidth: true
                    flat: true
                    label1.text: modelData.hasProfile ? modelData.connectionName : modelData.interfaceName
                    label1.elide: Text.ElideRight
                    label2.text: modelData.connected
                                 ? modelData.ipAddress.length > 0
                                   ? i18n("Connected · %1 · IP address %2", modelData.interfaceName, modelData.ipAddress)
                                   : i18n("Connected · %1", modelData.interfaceName)
                                 : modelData.connecting
                                   ? i18n("Connecting · %1", modelData.interfaceName)
                                   : i18n("Disconnected · %1", modelData.interfaceName)
                    label2.wrapMode: Text.Wrap
                    template.iconSource: "network-wired"
                    template.iconSizeHint: Maui.Style.iconSizes.small
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: !root.controller || root.controller.wiredConnections.length === 0
                flat: true
                label1.text: i18n("No wired connections")
                label1.elide: Text.ElideRight
                label2.text: i18n("Connect an Ethernet adapter or cable to use a wired network.")
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
        implicitHeight: _radioLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _radioLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Wireless Connectivity")
            description: root.controller && root.controller.available
                       ? i18n("Control the wireless radio and scan for networks.")
                       : i18n("No Wi-Fi adapter was found.")
            template.label2.wrapMode: Text.Wrap

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable WiFi")
                label1.elide: Text.ElideRight
                label2.text: root.controller && !root.controller.hardwareEnabled
                             ? i18n("Wi-Fi is disabled by a hardware switch.")
                             : i18n("Allow this computer to join wireless networks.")
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
                    checked: root.controller ? root.controller.wirelessEnabled : false
                    enabled: root.controller && root.controller.available && root.controller.hardwareEnabled
                    onToggled: if (root.controller) root.controller.wirelessEnabled = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Scan for networks")
                label1.elide: Text.ElideRight
                label2.text: root.controller && root.controller.scanning
                             ? i18n("Scanning for nearby wireless networks…")
                             : i18n("Refresh the list of nearby access points.")
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
                    text: i18n("Scan")
                    enabled: root.controller && root.controller.wirelessEnabled && !root.controller.scanning
                    onClicked: root.controller.requestScan()
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        visible: root.controller && root.controller.errorMessage.length > 0
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.negativeTextColor
        border.width: 1
        implicitHeight: _errorItem.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionItem
        {
            id: _errorItem
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            flat: true
            label1.text: i18n("Network error")
            label1.elide: Text.ElideRight
            label2.text: root.controller ? root.controller.errorMessage : ""
            label2.wrapMode: Text.Wrap
            template.content: ToolButton
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
                icon.name: "dialog-close"
                onClicked: if (root.controller) root.controller.clearError()
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
        implicitHeight: _networksLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _networksLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Available WiFi Networks")
            description: root.controller && root.controller.scanning
                       ? i18n("Scanning for wireless networks…")
                       : i18n("Networks are ordered by connection state and signal strength.")
            template.label2.wrapMode: Text.Wrap

            Repeater
            {
                id: _networksRepeater
                model: root.controller

                delegate: Maui.SectionItem
                {
                    required property string ssid
                    required property int signalStrength
                    required property string security
                    required property bool secure
                    required property bool passwordRequired
                    required property bool connected
                    required property bool saved
                    required property bool autoConnect
                    required property string devicePath
                    required property string accessPointPath

                    Layout.fillWidth: true
                    flat: true
                    label1.text: ssid
                    label1.elide: Text.ElideRight
                    label2.text: connected
                                 ? i18n("Connected · %1 · %2% signal", security, signalStrength)
                                 : saved
                                   ? i18n("Saved · %1 · %2% signal", security, signalStrength)
                                   : i18n("%1 · %2% signal", security, signalStrength)
                    label2.wrapMode: Text.Wrap
                    template.iconSource: secure ? "lock" : "unlock"
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
                            onClicked:
                            {
                                if (connected)
                                    root.controller.disconnectNetwork(devicePath)
                                else if (!passwordRequired || saved)
                                    root.controller.connectToNetwork(devicePath, accessPointPath, ssid, "")
                                else
                                {
                                    root.selectedDevicePath = devicePath
                                    root.selectedAccessPointPath = accessPointPath
                                    root.selectedSsid = ssid
                                    _passwordDialog.open()
                                }
                            }
                        }

                        ToolSeparator
                        {
                            topPadding: 10
                            bottomPadding: 10
                        }

                        ToolButton
                        {
                            icon.name: "configure"
                            enabled: saved
                            onClicked:
                            {
                                root.selectedDevicePath = devicePath
                                root.selectedSsid = ssid
                                root.selectedAutoConnect = autoConnect
                                _networkEditor.open()
                            }
                            ToolTip.visible: hovered && !saved
                            ToolTip.text: i18n("Connect once before editing this saved profile.")
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: root.controller && root.controller.wirelessEnabled && _networksRepeater.count === 0
                flat: true
                label1.text: i18n("No networks found")
                label1.elide: Text.ElideRight
                label2.text: i18n("Scan again or move closer to a wireless access point.")
                label2.wrapMode: Text.Wrap
            }
        }
    }

    Component.onCompleted: if (controller && controller.wirelessEnabled) controller.requestScan()
}
