import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.filebrowsing as FB

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var controller: (typeof networkController !== "undefined" && networkController) ? networkController : null
    property string selectedDevicePath: ""
    property string selectedAccessPointPath: ""
    property string selectedSsid: ""
    property bool selectedAutoConnect: false
    property string selectedConnectionUuid: ""
    property bool selectedIsWired: false
    property string vpnSecretConnectionPath: ""
    property string vpnSecretConnectionName: ""
    property list<string> vpnSecretHints: []
    property bool vpnSecretRequestHandled: false

    function clearVpnSecretFields()
    {
        _vpnUsernameField.clear()
        _vpnPasswordField.clear()
        _vpnCertificatePasswordField.clear()
        _vpnProxyPasswordField.clear()
    }

    function loadSelectedConnection()
    {
        if (!root.controller || root.selectedConnectionUuid.length === 0)
            return

        const details = root.controller.connectionDetails(root.selectedConnectionUuid)
        if (!details || !details.profileName)
            return

        _profileName.text = details.profileName
        _autoConnectSwitch.checked = !!details.autoconnect
        _ipv4Method.currentIndex = details.ipv4Method === "manual" ? 2 : (details.ipv4IgnoreAutoDns ? 1 : 0)
        _ipv4Addresses.text = details.ipv4Addresses || ""
        _ipv4Gateway.text = details.ipv4Gateway || ""
        _ipv4Dns.text = details.ipv4Dns || ""
        _ipv4SearchDomains.text = details.ipv4SearchDomains || ""
        _ipv6Method.currentIndex = details.ipv6Method === "disabled" ? 3 : (details.ipv6Method === "manual" ? 2 : (details.ipv6IgnoreAutoDns ? 1 : 0))
        _ipv6Addresses.text = details.ipv6Addresses || ""
        _ipv6Gateway.text = details.ipv6Gateway || ""
        _ipv6Dns.text = details.ipv6Dns || ""
        _ipv6SearchDomains.text = details.ipv6SearchDomains || ""
    }

    function saveSelectedConnection()
    {
        if (!root.controller || root.selectedConnectionUuid.length === 0)
            return
        const values = {
            id: _profileName.text,
            autoconnect: _autoConnectSwitch.checked,
            ipv4Method: _ipv4Method.currentIndex === 2 ? "manual" : "auto",
            ipv4IgnoreAutoDns: _ipv4Method.currentIndex === 1 || _ipv4Dns.text.trim().length > 0,
            ipv4Addresses: _ipv4Addresses.text,
            ipv4Gateway: _ipv4Gateway.text,
            ipv4Dns: _ipv4Dns.text,
            ipv4SearchDomains: _ipv4SearchDomains.text,
            ipv6Method: _ipv6Method.currentIndex === 3 ? "disabled" : (_ipv6Method.currentIndex === 2 ? "manual" : "auto"),
            ipv6IgnoreAutoDns: _ipv6Method.currentIndex === 1 || _ipv6Dns.text.trim().length > 0,
            ipv6Addresses: _ipv6Addresses.text,
            ipv6Gateway: _ipv6Gateway.text,
            ipv6Dns: _ipv6Dns.text,
            ipv6SearchDomains: _ipv6SearchDomains.text
        }
        root.controller.updateConnection(root.selectedConnectionUuid, values)
        _networkEditor.close()
    }

    function connectSelectedNetwork()
    {
        if (!root.controller || _passwordField.text.length === 0)
            return

        root.controller.connectToNetwork(root.selectedDevicePath, root.selectedAccessPointPath, root.selectedSsid, _passwordField.text)
        _passwordDialog.close()
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    FB.FileDialog
    {
        id: _openVpnDialog
        singleSelection: true
        searchBar: true
        mode: FB.FileDialog.Modes.Open
        currentPath: FB.FM.homePath()
        callback: (paths) =>
        {
            if (paths && paths.length && root.controller)
                root.controller.importOpenVpnConnection(paths[0])
        }
    }

    Maui.SettingsDialog
    {
        id: _vpnSecretsDialog
        title: i18n("VPN credentials")
        persistent: true

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small

            Label
            {
                Layout.fillWidth: true
                text: i18n("Enter the credentials required to connect to %1. These credentials are not saved.", root.vpnSecretConnectionName)
                color: Maui.Theme.textColor
                opacity: 0.7
                wrapMode: Text.WordWrap
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                visible: true
                label1.text: i18n("VPN username")
                label2.text: i18n("The username supplied by your VPN provider.")
                TextField
                {
                    id: _vpnUsernameField
                    Layout.fillWidth: true
                    placeholderText: i18n("Username")
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                visible: true
                label1.text: i18n("VPN password")
                label2.text: i18n("The password supplied by your VPN provider.")
                Maui.PasswordField
                {
                    id: _vpnPasswordField
                    Layout.fillWidth: true
                    placeholderText: i18n("Password")
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                visible: true
                label1.text: i18n("Certificate password")
                label2.text: i18n("The password for the certificate or private key.")
                Maui.PasswordField
                {
                    id: _vpnCertificatePasswordField
                    Layout.fillWidth: true
                    placeholderText: i18n("Certificate password")
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                visible: true
                label1.text: i18n("Proxy password")
                label2.text: i18n("The password supplied by your VPN proxy.")
                Maui.PasswordField
                {
                    id: _vpnProxyPasswordField
                    Layout.fillWidth: true
                    placeholderText: i18n("Proxy password")
                }
            }
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered:
                {
                    root.vpnSecretRequestHandled = true
                    if (root.controller)
                        root.controller.cancelVpnSecrets(root.vpnSecretConnectionPath)
                    _vpnSecretsDialog.close()
                }
            },
            Action
            {
                text: i18n("Connect")
                enabled: _vpnUsernameField.text.length > 0
                         || _vpnPasswordField.text.length > 0
                         || _vpnCertificatePasswordField.text.length > 0
                         || _vpnProxyPasswordField.text.length > 0
                onTriggered:
                {
                    const secrets = {}
                    if (_vpnUsernameField.text.length > 0)
                        secrets.username = _vpnUsernameField.text
                    if (_vpnPasswordField.text.length > 0)
                        secrets.password = _vpnPasswordField.text
                    if (_vpnCertificatePasswordField.text.length > 0)
                        secrets["cert-pass"] = _vpnCertificatePasswordField.text
                    if (_vpnProxyPasswordField.text.length > 0)
                    {
                        secrets["proxy-password"] = _vpnProxyPasswordField.text
                        secrets["http-proxy-password"] = _vpnProxyPasswordField.text
                    }
                    root.vpnSecretRequestHandled = true
                    if (root.controller)
                        root.controller.submitVpnSecrets(root.vpnSecretConnectionPath, secrets)
                    _vpnSecretsDialog.close()
                }
            }
        ]

        onClosed:
        {
            if (!root.vpnSecretRequestHandled && root.controller && root.vpnSecretConnectionPath.length > 0)
                root.controller.cancelVpnSecrets(root.vpnSecretConnectionPath)
            root.clearVpnSecretFields()
            root.vpnSecretConnectionPath = ""
            root.vpnSecretConnectionName = ""
            root.vpnSecretHints = []
            root.vpnSecretRequestHandled = false
        }
    }

    onVisibleChanged:
    {
        if (!visible)
        {
            _passwordDialog.close()
            _networkEditor.close()
            if (root.controller && root.vpnSecretConnectionPath.length > 0)
                root.controller.cancelVpnSecrets(root.vpnSecretConnectionPath)
            _vpnSecretsDialog.close()
        }
    }

    Maui.SettingsDialog
    {
        id: _passwordDialog
        title: i18n("Connect to %1", root.selectedSsid)
        persistent: true

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.medium

            RowLayout
            {
                Layout.fillWidth: true
                spacing: Maui.Style.space.medium

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

    Maui.SettingsDialog
    {
        id: _networkEditor
        title: i18n("Edit %1", root.selectedSsid)
        persistent: true

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small

            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("Profile name"); label2.text: i18n("The name shown for this saved NetworkManager connection."); TextField { id: _profileName; Layout.fillWidth: true; placeholderText: i18n("Profile name") } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("IPv4 configuration"); label2.text: i18n("Choose automatic addressing or enter a fixed IPv4 address below."); ComboBox { id: _ipv4Method; Layout.fillWidth: true; model: [i18n("Automatic"), i18n("Automatic (addresses only)"), i18n("Manual")]; displayText: currentText } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("IPv4 addresses"); label2.text: i18n("Enter one or more addresses using address/prefix notation, such as 192.168.1.20/24."); TextField { id: _ipv4Addresses; Layout.fillWidth: true; placeholderText: i18n("IPv4 addresses (address/prefix)") } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("IPv4 gateway"); label2.text: i18n("The router used to reach other networks."); TextField { id: _ipv4Gateway; Layout.fillWidth: true; placeholderText: i18n("IPv4 gateway") } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("IPv4 DNS servers"); label2.text: i18n("Optional name servers, separated by spaces or commas."); TextField { id: _ipv4Dns; Layout.fillWidth: true; placeholderText: i18n("IPv4 DNS servers") } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("IPv4 search domains"); label2.text: i18n("Optional domains used when resolving short host names."); TextField { id: _ipv4SearchDomains; Layout.fillWidth: true; placeholderText: i18n("IPv4 search domains") } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("IPv6 configuration"); label2.text: i18n("Choose automatic, manual, or disabled IPv6 addressing."); ComboBox { id: _ipv6Method; Layout.fillWidth: true; model: [i18n("Automatic"), i18n("Automatic (addresses only)"), i18n("Manual"), i18n("Disabled")]; displayText: currentText } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("IPv6 addresses"); label2.text: i18n("Enter IPv6 addresses using address/prefix notation, such as 2001:db8::20/64."); TextField { id: _ipv6Addresses; Layout.fillWidth: true; placeholderText: i18n("IPv6 addresses (address/prefix)") } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("IPv6 gateway"); label2.text: i18n("The IPv6 router used to reach other networks."); TextField { id: _ipv6Gateway; Layout.fillWidth: true; placeholderText: i18n("IPv6 gateway") } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("IPv6 DNS servers"); label2.text: i18n("Optional IPv6 name servers, separated by spaces or commas."); TextField { id: _ipv6Dns; Layout.fillWidth: true; placeholderText: i18n("IPv6 DNS servers") } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("IPv6 search domains"); label2.text: i18n("Optional IPv6 domains used when resolving short host names."); TextField { id: _ipv6SearchDomains; Layout.fillWidth: true; placeholderText: i18n("IPv6 search domains") } }

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
                    }
                }
            }
        }

        actions: [
            Action { text: i18n("Save"); onTriggered: root.saveSelectedConnection() },
            Action
            {
                text: i18n("Forget")
                enabled: !root.selectedIsWired
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

        ColumnLayout
        {
            id: _wiredLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Wired Connectivity")
                text2: root.controller && root.controller.wiredConnections.length > 0
                       ? i18n("Ethernet interfaces and saved wired connections.")
                       : i18n("No Ethernet interface was found.")
                label2.wrapMode: Text.Wrap
            }

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
                    template.content: RowLayout {
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
                        Button { text: modelData.connected ? i18n("Disconnect") : i18n("Connect"); onClicked: modelData.connected ? root.controller.disconnectNetwork(modelData.devicePath) : root.controller.connectWired(modelData.devicePath, modelData.connectionPath) }
                        ToolSeparator { topPadding: 10; bottomPadding: 10 }
                        ToolButton { icon.name: "configure"; enabled: modelData.hasProfile; onClicked: { root.selectedConnectionUuid = modelData.connectionUuid; root.selectedIsWired = true; root.selectedSsid = modelData.connectionName; root.selectedAutoConnect = modelData.autoConnect; _profileName.text = modelData.connectionName; root.loadSelectedConnection(); _networkEditor.open() } }
                    }
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

        ColumnLayout
        {
            id: _radioLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Wireless Connectivity")
                text2: root.controller && root.controller.available
                       ? i18n("Control the wireless radio and scan for networks.")
                       : i18n("No Wi-Fi adapter was found.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable WiFi")
                enabled: !!(root.controller && root.controller.available && root.controller.hardwareEnabled)
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
                    enabled: !!(root.controller && root.controller.available && root.controller.hardwareEnabled)
                    onToggled: if (root.controller) root.controller.wirelessEnabled = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Scan for networks")
                enabled: !!(root.controller && root.controller.wirelessEnabled && !root.controller.scanning)
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
                    enabled: !!(root.controller && root.controller.wirelessEnabled && !root.controller.scanning)
                    onClicked: root.controller.requestScan()
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
        implicitHeight: _networkSecurityLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _networkSecurityLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Network Security")
                text2: i18n("Protect DNS queries and import VPN connections.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("DNS encryption")
                enabled: root.controller !== null
                label1.elide: Text.ElideRight
                label2.text: i18n("Send DNS queries through the local dnscrypt-proxy resolver for all saved connections.")
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
                    checked: !!(root.controller && root.controller.dnsEncryptionEnabled)
                    enabled: root.controller !== null
                    onToggled:
                    {
                        if (root.controller)
                            root.controller.dnsEncryptionEnabled = checked
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Import OpenVPN connection")
                enabled: root.controller !== null
                label1.elide: Text.ElideRight
                label2.text: i18n("Create a NetworkManager VPN profile from an OpenVPN configuration file.")
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
                    text: i18n("Import")
                    enabled: root.controller !== null
                    onClicked: _openVpnDialog.open()
                }
            }

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                visible: !!(root.controller && root.controller.vpnConnections.length > 0)
                text1: i18n("VPN connections")
                text2: i18n("Connect to imported NetworkManager VPN profiles.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                model: root.controller ? root.controller.vpnConnections : []

                delegate: Maui.SectionItem
                {
                    required property var modelData

                    Layout.fillWidth: true
                    visible: !!modelData
                    flat: true
                    label1.text: modelData.connectionName
                    label1.elide: Text.ElideRight
                    label2.text: modelData.connected
                                 ? i18n("Connected")
                                 : modelData.connecting
                                   ? i18n("Connecting")
                                   : i18n("Disconnected")
                    label2.wrapMode: Text.Wrap
                    template.iconSource: "network-vpn"
                    template.content: Button
                    {
                        text: modelData.connected || modelData.connecting ? i18n("Disconnect") : i18n("Connect")
                        enabled: root.controller !== null
                        onClicked: modelData.connected || modelData.connecting
                                   ? root.controller.disconnectVpnConnection(modelData.connectionPath)
                                   : root.controller.connectVpnConnection(modelData.connectionPath)
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: !!(root.controller && root.controller.vpnConnections.length === 0)
                flat: true
                label1.text: i18n("No imported VPN connections")
                label1.elide: Text.ElideRight
                label2.text: i18n("Import an OpenVPN configuration file to add a VPN profile.")
                label2.wrapMode: Text.Wrap
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        visible: !!(root.controller && root.controller.connectedWirelessConnection && root.controller.connectedWirelessConnection.ssid && root.controller.connectedWirelessConnection.ssid.length > 0)
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _currentWifiLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _currentWifiLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Current WiFi Network")
                text2: i18n("The wireless network currently connected to this device.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                id: _currentWifiItem
                property var connection: root.controller ? root.controller.connectedWirelessConnection : ({})
                Layout.fillWidth: true
                visible: !!(_currentWifiItem.connection && _currentWifiItem.connection.ssid && _currentWifiItem.connection.ssid.length > 0)
                flat: true
                label1.text: _currentWifiItem.connection.ssid || ""
                label1.elide: Text.ElideRight
                label2.text: _currentWifiItem.connection.ssid ? i18n("Connected · %1 · %2% signal", _currentWifiItem.connection.security, _currentWifiItem.connection.signalStrength) : ""
                label2.wrapMode: Text.Wrap
                template.iconSource: _currentWifiItem.connection.security && _currentWifiItem.connection.security !== i18n("Open") ? "lock" : "unlock"
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
                    Button { text: i18n("Disconnect"); onClicked: root.controller.disconnectNetwork(_currentWifiItem.connection.devicePath) }
                    ToolSeparator { topPadding: 10; bottomPadding: 10 }
                    ToolButton
                    {
                        icon.name: "configure"
                        enabled: !!(_currentWifiItem.connection && _currentWifiItem.connection.saved)
                        onClicked:
                        {
                            root.selectedConnectionUuid = _currentWifiItem.connection.connectionUuid
                            root.selectedIsWired = false
                            root.selectedDevicePath = _currentWifiItem.connection.devicePath
                            root.selectedSsid = _currentWifiItem.connection.ssid
                            root.selectedAutoConnect = _currentWifiItem.connection.autoConnect
                            _profileName.text = _currentWifiItem.connection.ssid
                            root.loadSelectedConnection()
                            _networkEditor.open()
                        }
                    }
                }
            }
        }
    }

    Connections
    {
        target: root.controller

        function onVpnSecretsRequested(connectionPath, connectionName, hints)
        {
            if (_vpnSecretsDialog.visible)
            {
                root.vpnSecretRequestHandled = true
                root.clearVpnSecretFields()
            }
            root.vpnSecretConnectionPath = connectionPath
            root.vpnSecretConnectionName = connectionName
            root.vpnSecretHints = hints
            root.vpnSecretRequestHandled = false
            if (!_vpnSecretsDialog.visible)
                _vpnSecretsDialog.open()
        }

        function onVpnSecretsCanceled(connectionPath)
        {
            if (connectionPath === root.vpnSecretConnectionPath)
                _vpnSecretsDialog.close()
        }

        function onErrorMessageChanged()
        {
            if (!root.controller || root.controller.errorMessage.length === 0)
                return

            Maui.App.rootComponent.notify("dialog-error", i18n("Network error"), root.controller.errorMessage)
            root.controller.clearError()
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

        ColumnLayout
        {
            id: _networksLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Available WiFi Networks")
                text2: root.controller && root.controller.scanning
                       ? i18n("Scanning for wireless networks…")
                       : i18n("Networks are ordered by connection state and signal strength.")
                label2.wrapMode: Text.Wrap
            }

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
                    required property string connectionUuid

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
                                root.selectedConnectionUuid = connectionUuid
                                root.selectedIsWired = false
                                _profileName.text = ssid
                                root.loadSelectedConnection()
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
                visible: !!(root.controller && root.controller.wirelessEnabled && _networksRepeater.count === 0)
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
