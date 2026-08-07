import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    anchors.fill: parent
    spacing: Maui.Style.space.big

    readonly property var manager: (typeof dateTimeManager !== "undefined") ? dateTimeManager : null
    property date selectedDate: new Date()
    property int selectedHour: new Date().getHours()
    property int selectedMinute: new Date().getMinutes()
    property string savedDateTime: ""
    property string selectedHostName: ""
    property string savedHostName: ""
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
        }
    }

    Component.onCompleted: root.reloadSettings()

    property date tempDate: root.selectedDate
    property int tempHour: root.selectedHour
    property int tempMinute: root.selectedMinute

    Maui.SettingsDialog
    {
        id: dateDialog
        title: i18n("Select Date")
        persistent: true

        onOpened:
        {
            root.tempDate = root.selectedDate
            dateCalendar.showSelectedMonth()
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
            id: dateCalendar
            Layout.fillWidth: true
            implicitHeight: childrenRect.height
            spacing: Maui.Style.space.small

            property int displayedMonth: root.tempDate.getMonth()
            property int displayedYear: root.tempDate.getFullYear()
            readonly property int firstWeekday: (new Date(displayedYear, displayedMonth, 1).getDay() + 6) % 7
            readonly property int monthDays: new Date(displayedYear, displayedMonth + 1, 0).getDate()

            function showSelectedMonth()
            {
                displayedMonth = root.tempDate.getMonth()
                displayedYear = root.tempDate.getFullYear()
            }

            RowLayout
            {
                Layout.fillWidth: true

                Button
                {
                    text: "‹"
                    onClicked:
                    {
                        if (dateCalendar.displayedMonth === 0)
                        {
                            dateCalendar.displayedMonth = 11
                            --dateCalendar.displayedYear
                        }
                        else
                            --dateCalendar.displayedMonth
                    }
                }

                Label
                {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: Qt.formatDate(new Date(dateCalendar.displayedYear,
                                                  dateCalendar.displayedMonth, 1), "MMMM yyyy")
                    font.bold: true
                }

                Button
                {
                    text: "›"
                    onClicked:
                    {
                        if (dateCalendar.displayedMonth === 11)
                        {
                            dateCalendar.displayedMonth = 0
                            ++dateCalendar.displayedYear
                        }
                        else
                            ++dateCalendar.displayedMonth
                    }
                }
            }

            GridLayout
            {
                Layout.fillWidth: true
                columns: 7
                columnSpacing: Maui.Style.space.tiny
                rowSpacing: Maui.Style.space.tiny

                Repeater
                {
                    model: [i18n("Mon"), i18n("Tue"), i18n("Wed"), i18n("Thu"),
                            i18n("Fri"), i18n("Sat"), i18n("Sun")]

                    delegate: Label
                    {
                        required property string modelData
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: modelData
                        opacity: 0.7
                    }
                }

                Repeater
                {
                    model: 42

                    delegate: Button
                    {
                        required property int index
                        readonly property int dayNumber: index - dateCalendar.firstWeekday + 1
                        readonly property bool validDay: dayNumber > 0 && dayNumber <= dateCalendar.monthDays
                        readonly property bool selectedDay: validDay
                            && root.tempDate.getFullYear() === dateCalendar.displayedYear
                            && root.tempDate.getMonth() === dateCalendar.displayedMonth
                            && root.tempDate.getDate() === dayNumber

                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredHeight: Maui.Style.units.gridUnit * 2.5
                        text: validDay ? String(dayNumber) : ""
                        enabled: validDay
                        opacity: validDay ? 1 : 0
                        highlighted: selectedDay

                        onClicked: root.tempDate = new Date(dateCalendar.displayedYear,
                                                            dateCalendar.displayedMonth, dayNumber)
                    }
                }
            }
        }
    }

    Maui.SettingsDialog
    {
        id: timeDialog
        title: i18n("Select Time")
        persistent: true

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
        }
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("System")
        text2: i18n("Configure the system locale, timezone, date, and time.")
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
                text1: i18n("Date and time")
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
                    Layout.maximumWidth: Maui.Style.units.gridUnit * 18
                    text: root.selectedHostName
                    selectByMouse: true
                    onTextEdited: root.selectedHostName = text
                }
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
