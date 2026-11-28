import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.mauikit.filebrowsing as FB
import org.mauikit.calendar 1.0 as MauiCalendar

Maui.ScrollColumn
{
    id: root
    anchors.fill: parent
    spacing: Maui.Style.space.big

    readonly property var manager: (typeof systemManager !== "undefined") ? systemManager : null
    property date selectedDate: new Date()
    property int selectedHour: new Date().getHours()
    property int selectedMinute: new Date().getMinutes()
    property string savedDateTime: ""
    property string selectedHostName: ""
    property string savedHostName: ""
    property string originalGroups: ""
    property string editingUsername: ""
    property string editingAvatarPath: ""
    property string pendingDeleteUsername: ""
    property string avatarPath: ""

    function pickAvatar()
    {
        avatarDialog.currentPath = FB.FM.homePath()
        avatarDialog.callback = (paths) => { if (paths && paths.length) avatarPath = paths[0] }
        avatarDialog.open()
    }

    function openEditUserDialog(user) { editingUsername = user.username; editingAvatarPath = ""; profileName.text = user.name; profileEmail.text = user.email; profileGroups.text = user.groups; originalGroups = user.groups; editUserDialog.open() }
    function pickEditAvatar() { avatarDialog.currentPath = FB.FM.homePath(); avatarDialog.callback = (paths) => { if (paths && paths.length) editingAvatarPath = paths[0] }; avatarDialog.open() }
    function requestDeleteUser(username) { pendingDeleteUsername = username; deleteUserDialog.open() }
    function openAddUserDialog()
    {
        usernameField.clear()
        fullNameField.clear()
        passwordField.clear()
        avatarPath = ""
        addUserDialog.open()
    }
    readonly property string selectedDateTime: Qt.formatDate(selectedDate, "yyyy-MM-dd")
        + "T" + ("0" + selectedHour).slice(-2)
        + ":" + ("0" + selectedMinute).slice(-2) + ":00"
    readonly property bool saveAvailable: root.manager && !root.manager.busy
        && (root.selectedHostName !== root.savedHostName
            || (!automaticTimezoneSwitch.checked && root.selectedDateTime !== root.savedDateTime))

    property string timezoneFilter: ""
    property var filteredTimezones: manager
        ? manager.timezones.filter((zone) => zone.toLowerCase().indexOf(root.timezoneFilter.toLowerCase()) >= 0)
        : []

    function reloadSettings()
    {
        const now = new Date()
        root.selectedDate = now
        root.selectedHour = now.getHours()
        root.selectedMinute = now.getMinutes()
        root.savedDateTime = root.selectedDateTime
        root.selectedHostName = root.manager ? root.manager.hostName : ""
        root.savedHostName = root.selectedHostName
    }

    function saveSettings()
    {
        if (!root.saveAvailable)
            return false

        if (root.selectedHostName !== root.savedHostName)
            root.manager.setHostName(root.selectedHostName)
        if (!automaticTimezoneSwitch.checked && root.selectedDateTime !== root.savedDateTime)
            root.manager.setDateTime(root.selectedDateTime)
        return true
    }

    Connections
    {
        target: root.manager

        function onOperationSucceeded(message)
        {
            if (message === "System clock updated.")
                root.savedDateTime = root.selectedDateTime
            else if (message === "Hostname updated.")
            {
                root.savedHostName = root.selectedHostName
                if (!automaticTimezoneSwitch.checked && root.selectedDateTime !== root.savedDateTime)
                    root.manager.setDateTime(root.selectedDateTime)
            }
            else if (message === "User updated.") { root.manager.reloadUsers(); editUserDialog.close() }
            else if (message === "User deleted.") { root.manager.reloadUsers(); deleteUserDialog.close() }
            else if (message === "User added.")
            {
                root.manager.reloadUsers()
                addUserDialog.close()
            }
        }
    }

    FB.FileDialog
    {
        id: avatarDialog
        singleSelection: true
        searchBar: true
        mode: FB.FileDialog.Modes.Open
        currentPath: FB.FM.homePath()
    }

    FB.FileListingDialog
    {
        id: addUserDialog
        title: i18n("Add User")
        persistent: true

        template.visible: true
        template.iconVisible: false
        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: addUserDialog.close()
            },
            Action
            {
                text: i18n("Add")
                enabled: usernameField.text.length > 0 && (!administratorSwitch.checked || passwordField.text.length > 0) && root.manager && !root.manager.busy
                onTriggered: root.manager.addUser(usernameField.text, fullNameField.text, passwordField.text, administratorSwitch.checked, passwordQualitySwitch.checked, avatarPath)
            }
        ]

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small
            Label { Layout.fillWidth: true; text: i18n("Create a local user account. Administrator accounts require a password."); wrapMode: Text.WordWrap; opacity: 0.8 }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Avatar")
                label2.text: avatarPath.length > 0 ? avatarPath : i18n("Use the default account icon.")
                Button
                {
                    text: i18n("Choose")
                    onClicked: root.pickAvatar()
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Username")
                label2.text: i18n("Use lowercase letters, numbers, underscores, or hyphens.")
                TextField
                {
                    id: usernameField
                    Layout.fillWidth: true
                    placeholderText: i18n("username")
                    inputMethodHints: Qt.ImhLowercaseOnly
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Full name")
                label2.text: i18n("Optional name shown for this account.")
                TextField
                {
                    id: fullNameField
                    Layout.fillWidth: true
                    placeholderText: i18n("Full name (optional)")
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Administrator")
                label2.text: i18n("Allow this account to perform administrative tasks.")
                Switch
                {
                    id: administratorSwitch
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Password")
                label2.text: administratorSwitch.checked ? i18n("Required for administrator accounts.") : i18n("Optional for normal accounts.")
                TextField
                {
                    id: passwordField
                    Layout.fillWidth: true
                    placeholderText: administratorSwitch.checked ? i18n("Password (required)") : i18n("Password (optional)")
                    echoMode: TextInput.Password
                    passwordMaskDelay: 0
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Use secure password")
                enabled: passwordField.text.length > 0
                label2.text: i18n("Require strong passwords for user accounts.")
                Switch
                {
                    id: passwordQualitySwitch
                    checked: passwordField.text.length > 0
                    enabled: passwordField.text.length > 0
                }
            }
        }
    }

    FB.FileListingDialog
    {
        id: editUserDialog
        title: i18n("Configure User")
        persistent: true
        template.visible: true
        template.iconVisible: false
        actions: [Action { text: i18n("Cancel"); Maui.Controls.status: Maui.Controls.Normal; onTriggered: editUserDialog.close() }, Action { text: i18n("Apply"); Maui.Controls.status: Maui.Controls.Positive; enabled: root.manager && !root.manager.busy; onTriggered: root.manager.updateProfile(editingUsername, profileName.text, profileEmail.text, profileGroups.text, profileGroups.text !== originalGroups, editingAvatarPath) }]
        ColumnLayout { Layout.fillWidth: true; spacing: Maui.Style.space.small
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("Avatar"); label2.text: editingAvatarPath.length > 0 ? editingAvatarPath : i18n("Keep the current avatar."); Button { text: i18n("Choose"); onClicked: root.pickEditAvatar() } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("Real name"); TextField { id: profileName; Layout.fillWidth: true } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("Email"); TextField { id: profileEmail; Layout.fillWidth: true } }
            Maui.FlexSectionItem { Layout.fillWidth: true; label1.text: i18n("Supplementary groups (comma-separated)"); TextField { id: profileGroups; Layout.fillWidth: true } }
        }
    }

    FB.FileListingDialog
    {
        id: deleteUserDialog
        title: i18n("Delete User")
        persistent: true
        template.visible: true
        template.iconVisible: false
        actions: [Action { text: i18n("Cancel"); Maui.Controls.status: Maui.Controls.Normal; onTriggered: deleteUserDialog.close() }, Action { text: i18n("Delete"); Maui.Controls.status: Maui.Controls.Negative; onTriggered: root.manager.deleteUser(pendingDeleteUsername) }]
        Label { Layout.fillWidth: true; text: i18n("Delete the user account %1 and its home directory?", root.pendingDeleteUsername); wrapMode: Text.WordWrap }
    }

    Component.onCompleted: {
        if (root.manager)
            root.manager.initialize()
        root.reloadSettings()
    }

    property date tempDate: root.selectedDate
    property int tempHour: root.selectedHour
    property int tempMinute: root.selectedMinute

    FB.FileListingDialog
    {
        id: dateDialog
        title: i18n("Select Date")
        persistent: true

        template.visible: true
        template.iconVisible: false
        onOpened:
        {
            root.tempDate = root.selectedDate
            dateCalendar.setToDate(root.tempDate)
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: dateDialog.close()
            },
            Action
            {
                text: i18n("Accept")
                onTriggered:
                {
                    root.selectedDate = root.tempDate
                    dateDialog.close()
                }
            }
        ]

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small

            RowLayout
            {
                Layout.fillWidth: true

                Button
                {
                    text: "‹"
                    onClicked: dateCalendar.previousDate()
                }

                Label
                {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: Qt.formatDate(dateCalendar.firstDayOfMonth || root.tempDate, "MMMM yyyy")
                    font.bold: true
                }

                Button
                {
                    text: "›"
                    onClicked: dateCalendar.nextDate()
                }
            }

            MauiCalendar.MonthView
            {
                id: dateCalendar
                Layout.fillWidth: true
                Layout.preferredHeight: Maui.Style.units.gridUnit * 14
                selectedDate: root.tempDate
                dragDropEnabled: false
                onDateClicked: function(date) { root.tempDate = date }
            }
        }
    }

    FB.FileListingDialog
    {
        id: timeDialog
        title: i18n("Select Time")
        persistent: true

        template.visible: true
        template.iconVisible: false
        onOpened:
        {
            root.tempHour = root.selectedHour
            root.tempMinute = root.selectedMinute
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: timeDialog.close()
            },
            Action
            {
                text: i18n("Accept")
                onTriggered:
                {
                    root.selectedHour = root.tempHour
                    root.selectedMinute = root.tempMinute
                    timeDialog.close()
                }
            }
        ]

        RowLayout
        {
            id: timePicker
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            implicitHeight: Maui.Style.units.gridUnit * 12
            spacing: Maui.Style.space.small

            Item { Layout.fillWidth: true }

            Tumbler
            {
                id: hoursTumbler
                Layout.fillHeight: true
                Layout.preferredWidth: Maui.Style.units.gridUnit * 5
                model: 24
                currentIndex: root.tempHour

                delegate: Label
                {
                    required property int index
                    width: hoursTumbler.width
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: ("0" + index).slice(-2)
                    font.bold: index === hoursTumbler.currentIndex
                    opacity: index === hoursTumbler.currentIndex ? 1.0 : 0.4
                }

                onCurrentIndexChanged: root.tempHour = currentIndex
            }

            Label
            {
                text: ":"
                font.pointSize: Maui.Style.fontSizes.big
                font.bold: true
            }

            Tumbler
            {
                id: minutesTumbler
                Layout.fillHeight: true
                Layout.preferredWidth: Maui.Style.units.gridUnit * 5
                model: 60
                currentIndex: root.tempMinute

                delegate: Label
                {
                    required property int index
                    width: minutesTumbler.width
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: ("0" + index).slice(-2)
                    font.bold: index === minutesTumbler.currentIndex
                    opacity: index === minutesTumbler.currentIndex ? 1.0 : 0.4
                }

                onCurrentIndexChanged: root.tempMinute = currentIndex
            }

            Item { Layout.fillWidth: true }
        }
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("System")
        text2: i18n("Configure the system hostname, locale, timezone, clock, and user accounts.")
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _timezoneLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _timezoneLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Timezone")
                text2: i18n("Choose the timezone used by the system clock.")
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Locale")
                label2.text: i18n("Select the system language and regional format.")
                template.content: ComboBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    model: root.manager ? root.manager.locales : []
                    currentIndex: Math.max(0, root.manager ? root.manager.locales.indexOf(root.manager.locale) : -1)
                    onActivated: (index) => { if (root.manager) root.manager.setLocale(model[index]) }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("IANA timezone")
                label2.text: i18n("Select a standard timezone.")

                template.content: ComboBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    model: root.filteredTimezones
                    currentIndex: Math.max(0, root.filteredTimezones.indexOf(
                        root.manager ? root.manager.timezone : ""))
                    onActivated: (index) =>
                    {
                        if (root.manager)
                            root.manager.setTimezone(root.filteredTimezones[index])
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Automatic timezone lookup")
                label2.text: i18n("Use IP location to suggest the system timezone.")

                template.content: Switch
                {
                    id: automaticTimezoneSwitch
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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

                    checked: root.manager ? root.manager.automaticLocation : false
                    onToggled:
                    {
                        if (root.manager)
                            root.manager.automaticLocation = checked
                    }
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
        implicitHeight: _dateTimeLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _dateTimeLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Date and Time")
                text2: i18n("Set the system date and time manually when offline.")
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: !automaticTimezoneSwitch.checked && root.manager && !root.manager.busy
                label1.text: i18n("Date")
                label2.text: i18n("Choose the calendar date.")

                template.content: Button
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    text: Qt.formatDate(root.selectedDate, "ddd, MMM d, yyyy")
                    onClicked:
                    {
                        dateDialog.open()
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: !automaticTimezoneSwitch.checked && root.manager && !root.manager.busy
                label1.text: i18n("Time")
                label2.text: i18n("Choose the time.")

                template.content: Button
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    text: Qt.formatTime(new Date(0, 0, 0, root.selectedHour, root.selectedMinute), "hh:mm")
                    onClicked:
                    {
                        root.tempHour = root.selectedHour
                        root.tempMinute = root.selectedMinute
                        timeDialog.open()
                    }
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
        implicitHeight: _hostnameLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _hostnameLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Hostname")
                text2: i18n("Choose the name used to identify this computer on the network.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: root.manager && !root.manager.busy
                label1.text: i18n("Computer name")
                label2.text: i18n("Use letters, numbers, dots, and hyphens.")
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
                    text: root.selectedHostName
                    selectByMouse: true
                    onTextEdited: root.selectedHostName = text
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
        implicitHeight: _usersLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _usersLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Users")
                text2: i18n("Manage the user accounts available on this computer.")
            }

            Repeater
            {
                model: root.manager ? root.manager.users : []

                delegate: Maui.SectionItem
                {
                    required property var modelData
                    Layout.fillWidth: true
                    flat: true
                    label1.text: modelData.name.length > 0 ? modelData.name : modelData.username
                    label1.elide: Text.ElideRight
                    label2.text: modelData.username + " • " + modelData.home
                        + (root.manager.pinEnabled && modelData.username === root.manager.credentialUser()
                           ? " • " + i18n("PIN enabled") : "")
                    label2.elide: Text.ElideRight
                    label2.wrapMode: Text.NoWrap
                    template.imageSource: modelData.facePath || ""
                    template.content: Row
                    {
                        spacing: Maui.Style.space.tiny
                        ToolButton { icon.name: "document-edit"; display: AbstractButton.IconOnly; ToolTip.text: i18n("Configure user"); onClicked: root.openEditUserDialog(modelData) } ToolButton { visible: modelData.canDelete; icon.name: "edit-delete"; display: AbstractButton.IconOnly; ToolTip.text: i18n("Delete user"); onClicked: root.requestDeleteUser(modelData.username) }
                    }

                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: !root.manager || root.manager.users.length === 0
                flat: true
                label1.text: i18n("No user accounts found")
                label2.text: i18n("No regular local accounts are available.")
                template.iconSource: "user-identity"
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        implicitHeight: credentialCard.implicitHeight + Maui.Style.contentMargins * 2
        ColumnLayout {
            id: credentialCard
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small
            readonly property string operation: ["setup", "change", "delete"][method.currentIndex]

            function resetFields()
            {
                if (oldPin) oldPin.clear()
                if (newPin) newPin.clear()
                if (confirmPin) confirmPin.clear()
                if (status) status.text = ""
            }

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Authentication")
                text2: root.manager
                    ? i18n("Manage authentication credentials for %1.", root.manager.credentialUser())
                    : i18n("Manage PIN and fingerprint credentials for this user.")
            }

            Maui.FlexSectionItem {
                Layout.fillWidth: true
                label1.text: i18n("Password")
                label2.text: i18n("Change the sign-in password for a local account.")
                Button {
                    text: i18n("Change Password")
                    onClicked: passwordDialog.open()
                }
            }

            Maui.FlexSectionItem {
                Layout.fillWidth: true
                label1.text: i18n("PIN")
                label2.text: i18n("Set, change, or remove your PIN.")
                Button {
                    text: i18n("Manage")
                    onClicked: pinDialog.open()
                }
            }
            Maui.FlexSectionItem {
                Layout.fillWidth: true
                label1.text: i18n("Fingerprints")
                label2.text: fingerprintDialog.available
                    ? i18np("%1 enrolled fingerprint", "%1 enrolled fingerprints", fingerprintDialog.fingers.length)
                    : i18n("Manage your enrolled fingerprints.")
                Button {
                    text: i18n("Manage")
                    onClicked: fingerprintDialog.open()
                }
            }

        }
    }

    Maui.InfoDialog {
        id: passwordDialog
        title: passwordUser.currentText
            ? i18n("Change Password for %1", passwordUser.currentText) : i18n("Change Password")
        modal: true
        standardButtons: Dialog.Cancel | Dialog.Apply
        property string feedback: ""

        function clearForm() {
            accountPassword.clear()
            confirmPassword.clear()
            feedback = ""
        }

        onOpened: {
            passwordUser.currentIndex = passwordUser.find(root.manager.credentialUser())
            if (passwordUser.currentIndex < 0 && passwordUser.count) passwordUser.currentIndex = 0
            accountPassword.forceActiveFocus()
        }
        onClosed: clearForm()
        Component.onCompleted: {
            const apply = standardButton(Dialog.Apply)
            apply.text = i18n("Change Password")
            apply.enabled = Qt.binding(() => root.manager && !root.manager.busy && passwordUser.currentIndex >= 0
                && accountPassword.length > 0 && accountPassword.text === confirmPassword.text)
        }
        onApplied: {
            feedback = ""
            root.manager.updateUser(passwordUser.currentText, accountPassword.text, "")
            feedback = root.manager.errorMessage
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Maui.Style.space.medium
            Maui.FlexSectionItem {
                Layout.fillWidth: true
                wide: false
                label1.text: i18n("Local account")
                label2.text: i18n("Choose whose password to change.")
                ComboBox {
                    id: passwordUser
                    popup.enter: Transition {
                        PropertyAction { property: "scale"; value: 1 }
                        NumberAnimation {
                            property: "opacity"
                            from: 0
                            to: 1
                            duration: Maui.Style.enableEffects ? Maui.Style.units.shortDuration : 0
                        }
                    }
                    Layout.fillWidth: true
                    enabled: root.manager && !root.manager.busy
                    model: {
                        if (!root.manager) return []
                        const current = root.manager.credentialUser()
                        const accounts = Array.from(root.manager.users, user => user.username)
                        return current ? [current, ...accounts.filter(name => name !== current)] : accounts
                    }
                    onCurrentIndexChanged: if (confirmPassword) passwordDialog.clearForm()
                }
            }
            Maui.FlexSectionItem {
                Layout.fillWidth: true
                wide: false
                label1.text: i18n("New password")
                label2.text: i18n("Enter a new sign-in password.")
                Maui.PasswordField {
                    id: accountPassword
                    passwordMaskDelay: 0
                    actions: []
                    Layout.fillWidth: true
                }
            }
            RowLayout {
                id: strength
                Layout.fillWidth: true
                visible: accountPassword.length > 0
                readonly property int score: visible && root.manager
                    ? root.manager.passwordScore(accountPassword.text, passwordUser.currentText) : -1
                readonly property int level: score < 34 ? 1 : score < 67 ? 2 : 3
                readonly property color tint: level === 1 ? Maui.Theme.negativeTextColor
                    : level === 2 ? Maui.Theme.neutralTextColor : Maui.Theme.positiveTextColor
                ProgressBar {
                    Layout.fillWidth: true
                    to: 3
                    value: strength.level
                    id: strengthBar
                    contentItem: Rectangle {
                        implicitHeight: Maui.Style.space.small
                        radius: height / 2
                        color: Maui.Theme.alternateBackgroundColor
                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            x: strengthBar.mirrored ? parent.width - width : 0
                            width: parent.width * strengthBar.position
                            height: parent.height
                            radius: parent.radius
                            color: strength.tint
                        }
                    }
                    background: null
                    Accessible.name: i18n("Password strength")
                }
                Label {
                    color: strength.tint
                    text: strength.level === 1 ? i18n("Weak")
                        : strength.level === 2 ? i18n("Medium") : i18n("Strong")
                }
            }
            Maui.FlexSectionItem {
                Layout.fillWidth: true
                wide: false
                label1.text: i18n("Confirm password")
                label2.text: i18n("Enter the new password again.")
                Maui.PasswordField {
                    id: confirmPassword
                    passwordMaskDelay: 0
                    actions: []
                    Layout.fillWidth: true
                }
            }
            Label {
                Layout.fillWidth: true
                visible: confirmPassword.length > 0 && confirmPassword.text !== accountPassword.text
                text: i18n("Passwords do not match.")
                color: Maui.Theme.negativeTextColor
                wrapMode: Text.WordWrap
            }
            Label {
                Layout.fillWidth: true
                visible: text.length > 0
                wrapMode: Text.WordWrap
                color: Maui.Theme.negativeTextColor
                text: passwordDialog.feedback
            }
            Connections {
                target: root.manager
                function onErrorMessageChanged() {
                    if (passwordDialog.visible) passwordDialog.feedback = root.manager.errorMessage
                }
                function onOperationSucceeded(message) {
                    if (passwordDialog.visible && message === "User updated.") passwordDialog.close()
                }
            }
        }
    }

    Maui.InfoDialog {
        id: fingerprintDialog
        title: step !== "list" ? selectedName : root.manager
            ? i18n("Fingerprints for %1", root.manager.credentialUser()) : i18n("Fingerprints")
        modal: true
        standardButtons: Dialog.Close | Dialog.Apply
        property bool readerAvailable: false
        property var fingers: []
        property bool available: false
        property string step: "list"
        property string selectedName
        property string scanType
        property string feedback
        property int acceptedScans
        property int totalScans
        readonly property var fingerChoices: [
            { key: "right-index-finger", name: i18n("Right index finger") },
            { key: "left-thumb", name: i18n("Left thumb") },
            { key: "left-index-finger", name: i18n("Left index finger") },
            { key: "left-middle-finger", name: i18n("Left middle finger") },
            { key: "left-ring-finger", name: i18n("Left ring finger") },
            { key: "left-little-finger", name: i18n("Left little finger") },
            { key: "right-thumb", name: i18n("Right thumb") },
            { key: "right-middle-finger", name: i18n("Right middle finger") },
            { key: "right-ring-finger", name: i18n("Right ring finger") },
            { key: "right-little-finger", name: i18n("Right little finger") }
        ]
        readonly property var remaining: fingerChoices.filter(finger => !fingers.includes(finger.key))

        readonly property bool removingOne: fingerprintAction.currentIndex === 1 && fingers.length > 0

        readonly property bool removingAll: fingerprintAction.currentIndex === 2 && fingers.length > 0

        function fingerName(key) {
            const finger = fingerChoices.find(item => item.key === key)
            return finger ? finger.name : key
        }

        function showList() {
            if (root.manager) {
                root.manager.cancelFingerprintEnrollment()
                root.manager.refreshFingerprints()
            }
            feedback = ""
            fingerprintAction.currentIndex = 0
            step = "list"
        }

        function enroll(finger) {
            selectedName = finger.name
            acceptedScans = 0
            totalScans = 0
            scanType = ""
            feedback = ""
            step = "starting"
            root.manager.enrollFingerprint(finger.key)
        }

        onOpened: showList()
        onAboutToHide: if (root.manager) root.manager.cancelFingerprintEnrollment()
        Component.onCompleted: {
            standardButton(Dialog.Close).visible = Qt.binding(() => step === "list")
            const action = standardButton(Dialog.Apply)
            action.text = Qt.binding(() => {
                switch (step) {
                case "list": return removingAll ? i18n("Remove") : fingerprintAction.currentText
                case "complete": return i18n("Done")
                case "failed": return i18n("Back")
                default: return i18n("Cancel")
                }
            })
            action.Maui.Controls.status = Qt.binding(() => step === "list" && (removingOne || removingAll)
                ? Maui.Controls.Negative : Maui.Controls.Positive)
            action.enabled = Qt.binding(() => step !== "list" || (root.manager && !root.manager.busy && available
                && (removingAll ? root.manager.credentialToolAvailable("removePrints") : fingerSelector.currentIndex >= 0)))
            if (root.manager) root.manager.refreshFingerprints()
        }
        onApplied: {
            if (step !== "list") {
                showList()
            } else if (removingOne) {
                feedback = ""
                root.manager.removeFingerprint(fingers[fingerSelector.currentIndex])
            } else if (removingAll) {
                feedback = ""
                root.manager.manageCredential("removePrints", "", "")
            } else {
                enroll(remaining[fingerSelector.currentIndex])
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Maui.Style.space.medium
            Maui.FlexSectionItem {
                Layout.fillWidth: true
                wide: false
                visible: fingerprintDialog.step === "list"
                label1.text: i18n("Fingerprint action")
                label2.text: {
                    if (!fingerprintDialog.readerAvailable) return i18n("No fingerprint reader available.")
                    if (!fingerprintDialog.available) return i18n("Fingerprint list unavailable.")
                    if (!fingerprintDialog.fingers.length) return i18n("No fingerprints enrolled. Choose a finger below to add your first fingerprint.")
                    return i18n("Enrolled: %1", fingerprintDialog.fingers.map(key => fingerprintDialog.fingerName(key)).join(i18n(", ")))
                }
                ComboBox {
                    id: fingerprintAction
                    Layout.fillWidth: true
                    enabled: root.manager && !root.manager.busy && fingerprintDialog.available
                    model: fingerprintDialog.fingers.length
                        ? [i18n("Enroll Fingerprint"), i18n("Remove Fingerprint"), i18n("Remove All Fingerprints")] : [i18n("Enroll Fingerprint")]
                }
            }
            Item {
                Layout.fillWidth: true
                implicitHeight: Math.max(fingerField.implicitHeight, removalWarning.visible ? removalWarning.implicitHeight : 0)
                visible: fingerprintDialog.step === "list" && fingerprintDialog.available
                    && (fingerprintDialog.remaining.length > 0 || fingerprintDialog.removingOne || fingerprintDialog.removingAll)

                Maui.FlexSectionItem {
                    id: removalWarning
                    width: parent.width
                    wide: false
                    visible: fingerprintDialog.removingAll
                    label1.text: i18n("All fingerprints for this account will be deleted.")
                    label1.color: Maui.Theme.negativeTextColor
                    label2.text: i18n("You’ll need to enroll a finger again to use fingerprint authentication.")
                    label1.wrapMode: Text.WordWrap
                    label2.wrapMode: Text.WordWrap
                }
                Maui.FlexSectionItem {
                    id: fingerField
                    width: parent.width
                    wide: false
                    visible: !fingerprintDialog.removingAll
                    label1.text: fingerprintDialog.removingOne ? i18n("Finger to remove") : i18n("Finger to enroll")
                    label2.text: fingerprintDialog.removingOne ? i18n("Choose the enrolled fingerprint to remove.") : i18n("Choose the finger you will scan.")
                    ComboBox {
                        id: fingerSelector
                        Layout.fillWidth: true
                        model: fingerprintDialog.removingOne ? fingerprintDialog.fingers.map(key => fingerprintDialog.fingerName(key))
                                : fingerprintDialog.remaining.map(finger => finger.name)
                        Accessible.name: fingerField.label1.text
                    }
                }
            }
            Label {
                Layout.fillWidth: true
                visible: fingerprintDialog.step === "list" && text.length > 0
                text: fingerprintDialog.feedback
                wrapMode: Text.WordWrap
            }

            Maui.GridItemTemplate {
                Layout.fillWidth: true
                visible: fingerprintDialog.step !== "list"
                iconComponent: Component {
                    Item {
                        implicitHeight: Maui.Style.iconSizes.big
                        implicitWidth: implicitHeight
                        Canvas {
                            anchors.centerIn: parent
                            width: Math.min(parent.width, Maui.Style.iconSizes.big)
                            height: width
                            readonly property color ink: Maui.Theme.textColor
                            onInkChanged: requestPaint()
                            onPaint: {
                                const ctx = getContext("2d")
                                ctx.reset()
                                // Normalized ridge coordinates scale with the theme icon size.
                                ctx.scale(width, height)
                                ctx.strokeStyle = ink
                                ctx.lineWidth = 0.045
                                ctx.lineCap = "round"
                                ctx.beginPath()
                                ctx.moveTo(0.14, 0.30)
                                ctx.bezierCurveTo(0.31, 0.04, 0.68, 0.03, 0.86, 0.30)
                                ctx.moveTo(0.09, 0.53)
                                ctx.bezierCurveTo(0.10, 0.14, 0.77, 0.04, 0.89, 0.51)
                                ctx.moveTo(0.12, 0.73)
                                ctx.bezierCurveTo(0.24, 0.58, 0.13, 0.36, 0.43, 0.30)
                                ctx.bezierCurveTo(0.69, 0.24, 0.79, 0.44, 0.81, 0.62)
                                ctx.moveTo(0.22, 0.82)
                                ctx.bezierCurveTo(0.40, 0.63, 0.23, 0.45, 0.46, 0.41)
                                ctx.bezierCurveTo(0.71, 0.36, 0.64, 0.70, 0.84, 0.76)
                                ctx.moveTo(0.32, 0.89)
                                ctx.bezierCurveTo(0.52, 0.69, 0.36, 0.54, 0.49, 0.52)
                                ctx.bezierCurveTo(0.59, 0.51, 0.53, 0.75, 0.74, 0.85)
                                ctx.moveTo(0.49, 0.64)
                                ctx.bezierCurveTo(0.49, 0.78, 0.54, 0.87, 0.63, 0.93)
                                ctx.stroke()
                            }
                        }
                    }
                }
                label1.font: Maui.Style.h1Font
                label2.wrapMode: Text.WordWrap
                labelSizeHint: label1.implicitHeight + label2.implicitHeight + Maui.Style.space.medium
                text1: {
                    if (fingerprintDialog.step === "complete") return i18n("100%")
                    if (fingerprintDialog.step === "scanning" && fingerprintDialog.totalScans > 0)
                        return i18n("%1%", Math.min(99, Math.floor(100 * fingerprintDialog.acceptedScans / fingerprintDialog.totalScans)))
                    return ""
                }
                text2: {
                    if (fingerprintDialog.feedback) return fingerprintDialog.feedback
                    switch (fingerprintDialog.step) {
                    case "starting": return i18n("Preparing the reader…")
                    case "scanning":
                        if (fingerprintDialog.scanType === "swipe") return i18n("Swipe your finger across the reader, then repeat.")
                        if (fingerprintDialog.scanType === "press") return i18n("Touch the reader, then lift your finger and repeat.")
                        return i18n("Scan your finger again until enrollment is complete.")
                    case "complete": return i18n("Fingerprint added.")
                    default: return ""
                    }
                }
            }
            ProgressBar {
                Layout.fillWidth: true
                visible: fingerprintDialog.step === "scanning" && fingerprintDialog.totalScans > 0
                indeterminate: false
                to: Math.max(1, fingerprintDialog.totalScans)
                value: fingerprintDialog.acceptedScans
            }
            Connections {
                target: root.manager
                function onFingerprintReaderChanged(available) { fingerprintDialog.readerAvailable = available }
                function onFingerprintsListed(fingers, available) {
                    fingerprintDialog.fingers = fingers
                    fingerprintDialog.available = available
                }
                function onEnrollmentReady(scanType, stages) {
                    fingerprintDialog.scanType = scanType
                    fingerprintDialog.totalScans = stages
                    fingerprintDialog.step = "scanning"
                }
                function onEnrollmentScanAccepted() {
                    fingerprintDialog.acceptedScans++
                    fingerprintDialog.feedback = ""
                }
                function onEnrollmentFeedback(message) { fingerprintDialog.feedback = message }
                function onEnrollmentFinished(success, message) {
                    fingerprintDialog.feedback = message
                    fingerprintDialog.step = success ? "complete" : "failed"
                }
                function onCredentialMessage(message) {
                    if (fingerprintDialog.visible && fingerprintDialog.step === "list") fingerprintDialog.feedback = message
                }
            }
        }
    }

    Maui.InfoDialog {
        id: pinDialog
        title: root.manager ? i18n("PIN for %1", root.manager.credentialUser())
                            : i18n("PIN")
        modal: true
        standardButtons: Dialog.Close | Dialog.Apply
        onClosed: credentialCard.resetFields()
        Component.onCompleted: {
            const apply = standardButton(Dialog.Apply)
            apply.text = Qt.binding(() => method.currentText)
            apply.Maui.Controls.status = Qt.binding(() => credentialCard.operation === "delete"
                ? Maui.Controls.Negative : Maui.Controls.Positive)
            apply.enabled = Qt.binding(() => root.manager && !root.manager.busy && root.manager.credentialToolAvailable(credentialCard.operation)
                && (!oldPin.visible || oldPin.acceptableInput)
                && (!newPin.visible || (newPin.acceptableInput && newPin.text === confirmPin.text)))
        }
        onApplied: {
            status.text = ""
            root.manager.manageCredential(credentialCard.operation, oldPin.text, newPin.text)
            oldPin.clear()
            newPin.clear()
            confirmPin.clear()
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Maui.Style.space.medium
            Maui.FlexSectionItem {
                Layout.fillWidth: true
                wide: false
                label1.text: i18n("PIN action")
                label2.text: i18n("Choose how to manage your PIN.")
                ComboBox
                {
                    id: method
                    popup.enter: Transition {
                        PropertyAction { property: "scale"; value: 1 }
                        NumberAnimation {
                            property: "opacity"
                            from: 0
                            to: 1
                            duration: Maui.Style.enableEffects ? Maui.Style.units.shortDuration : 0
                        }
                    }
                    Layout.fillWidth: true
                    enabled: root.manager && !root.manager.busy
                    model: [i18n("Set PIN"), i18n("Change PIN"), i18n("Remove PIN")]
                    onCurrentIndexChanged: credentialCard.resetFields()
                }
            }
            Maui.FlexSectionItem {
                Layout.fillWidth: true
                wide: false
                visible: method.currentIndex === 1 || method.currentIndex === 2
                label1.text: i18n("Current PIN")
                label2.text: i18n("Enter the PIN you use now.")
                Maui.PasswordField {
                    id: oldPin
                    passwordMaskDelay: 0
                    actions: []
                    Layout.fillWidth: true
                    validator: RegularExpressionValidator { regularExpression: /[0-9]+/ }
                }
            }
            Maui.FlexSectionItem {
                Layout.fillWidth: true
                wide: false
                visible: method.currentIndex < 2
                label1.text: i18n("New PIN")
                label2.text: i18n("Enter a new numeric PIN.")
                Maui.PasswordField {
                    id: newPin
                    passwordMaskDelay: 0
                    actions: []
                    Layout.fillWidth: true
                    validator: RegularExpressionValidator { regularExpression: /[0-9]+/ }
                }
            }
            Maui.FlexSectionItem {
                Layout.fillWidth: true
                wide: false
                visible: method.currentIndex < 2
                label1.text: i18n("Confirm PIN")
                label2.text: i18n("Enter the new PIN again.")
                Maui.PasswordField {
                    id: confirmPin
                    passwordMaskDelay: 0
                    actions: []
                    Layout.fillWidth: true
                    validator: RegularExpressionValidator { regularExpression: /[0-9]+/ }
                }
            }
            Label {
                Layout.fillWidth: true
                visible: method.currentIndex < 2 && confirmPin.length > 0 && confirmPin.text !== newPin.text
                text: i18n("PINs do not match.")
                color: Maui.Theme.negativeTextColor
                wrapMode: Text.WordWrap
            }
            Label
            {
                Layout.fillWidth: true
                visible: root.manager && !root.manager.credentialToolAvailable(credentialCard.operation)
                text: i18n("PIN setup requires pinpam.")
                wrapMode: Text.WordWrap
            }

            Label
            {
                id: status
                visible: text.length > 0
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            Connections
            {
                target: root.manager
                function onPinOperationSucceeded() { pinDialog.close() }
                function onCredentialMessage(message) { if (pinDialog.visible) status.text = message }
            }
        }
    }


    Label
    {
        Layout.fillWidth: true
        visible: root.manager && root.manager.errorMessage.length > 0
        text: root.manager ? root.manager.errorMessage : ""
        color: Maui.Theme.negativeTextColor
        wrapMode: Text.WordWrap
    }
}
