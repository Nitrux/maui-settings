import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var kde: (typeof kdeGlobalsInfo !== "undefined" && kdeGlobalsInfo) ? kdeGlobalsInfo : null
    readonly property var hyprsunset: (typeof hyprsunsetInfo !== "undefined" && hyprsunsetInfo) ? hyprsunsetInfo : null
    readonly property var theme: (typeof themeInfo !== "undefined" && themeInfo) ? themeInfo : null
    readonly property bool scrollBarOnLeftAvailable: theme !== null && typeof theme.scrollBarOnLeft === "boolean"

    property bool stagedSingleClick: true
    property bool stagedScrollBarOnLeft: false
    property bool editingDaytimeProfile: true
    property int tempHour: 7
    property int tempMinute: 0

    function prepareResponsive(control)
    {
        const originalParent = control.parent
        control.responsiveSectionItem = originalParent.parent.parent.parent
        control.wideParent = originalParent
        control.updateResponsiveParent()
    }

    function reloadSettings()
    {
        if (kde)
        {
            kde.reload()
            stagedSingleClick = kde.singleClick
        }

        if (scrollBarOnLeftAvailable)
            stagedScrollBarOnLeft = theme.scrollBarOnLeft

        if (hyprsunset)
            hyprsunset.reload()
    }

    function saveSettings()
    {
        let saved = true

        if (kde)
        {
            kde.singleClick = stagedSingleClick
            saved = kde.save()
        } else {
            saved = false
        }

        if (scrollBarOnLeftAvailable)
            theme.scrollBarOnLeft = stagedScrollBarOnLeft
        else
            saved = false

        if (hyprsunset && hyprsunset.available)
            saved = hyprsunset.save() && saved

        return saved
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Accessibility")
        text2: i18n("Configure interaction preferences shared by MauiKit and KDE applications.")
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _interactionLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _interactionLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Interaction")
                text2: i18n("Choose how items are activated and vertical scroll bars are positioned.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Single-click activation")
                label1.elide: Text.ElideRight
                label2.text: i18n("Open files and folders with one click instead of a double-click.")
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
                    checked: root.stagedSingleClick
                    enabled: root.kde !== null
                    onToggled: root.stagedSingleClick = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Terminal scroll bars placement")
                label1.elide: Text.ElideRight
                label2.text: i18n("Show vertical scroll bars on the left side instead of the right side on all terminals.")
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
                    checked: root.stagedScrollBarOnLeft
                    enabled: root.scrollBarOnLeftAvailable
                    onToggled: root.stagedScrollBarOnLeft = checked
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
        implicitHeight: _nightLightLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _nightLightLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            enabled: root.hyprsunset && root.hyprsunset.available
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Night light")
                text2: !root.hyprsunset || !root.hyprsunset.available
                    ? i18n("hyprsunset is not available.")
                    : i18n("Configure the daytime and nighttime profiles.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Maximum gamma")
                label1.elide: Text.ElideRight
                label2.text: i18n("Limit the maximum gamma value hyprsunset can apply.")
                label2.wrapMode: Text.Wrap

                template.content: SpinBox
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
                    Component.onCompleted: root.prepareResponsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    from: 0
                    to: 200
                    value: root.hyprsunset ? root.hyprsunset.maxGamma : 150
                    onValueModified: if (root.hyprsunset) root.hyprsunset.maxGamma = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Daytime profile starts")
                label1.elide: Text.ElideRight
                label2.text: i18n("Switch to the daytime profile at this time.")
                label2.wrapMode: Text.Wrap

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
                    text: Qt.formatTime(new Date(0, 0, 0,
                                                 root.hyprsunset ? root.hyprsunset.daytimeHour : 7,
                                                 root.hyprsunset ? root.hyprsunset.daytimeMinute : 0), "hh:mm")
                    onClicked:
                    {
                        root.editingDaytimeProfile = true
                        root.tempHour = root.hyprsunset ? root.hyprsunset.daytimeHour : 7
                        root.tempMinute = root.hyprsunset ? root.hyprsunset.daytimeMinute : 0
                        timeDialog.open()
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Daytime identity mode")
                label1.elide: Text.ElideRight
                label2.text: i18n("Use the normal screen temperature during the daytime profile.")
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
                    Component.onCompleted: root.prepareResponsive(this)
                    checked: root.hyprsunset ? root.hyprsunset.daytimeIdentity : true
                    onToggled: if (root.hyprsunset) root.hyprsunset.daytimeIdentity = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Nighttime profile starts")
                label1.elide: Text.ElideRight
                label2.text: i18n("Switch to the nighttime profile at this time.")
                label2.wrapMode: Text.Wrap

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
                    text: Qt.formatTime(new Date(0, 0, 0,
                                                 root.hyprsunset ? root.hyprsunset.nighttimeHour : 19,
                                                 root.hyprsunset ? root.hyprsunset.nighttimeMinute : 0), "hh:mm")
                    onClicked:
                    {
                        root.editingDaytimeProfile = false
                        root.tempHour = root.hyprsunset ? root.hyprsunset.nighttimeHour : 19
                        root.tempMinute = root.hyprsunset ? root.hyprsunset.nighttimeMinute : 0
                        timeDialog.open()
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Nighttime temperature")
                label1.elide: Text.ElideRight
                label2.text: i18n("Lower values produce a warmer screen.")
                label2.wrapMode: Text.Wrap

                template.content: SpinBox
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
                    Component.onCompleted: root.prepareResponsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    from: 1000
                    to: 10000
                    stepSize: 100
                    value: root.hyprsunset ? root.hyprsunset.nighttimeTemperature : 5800
                    onValueModified: if (root.hyprsunset) root.hyprsunset.nighttimeTemperature = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Nighttime gamma")
                label1.elide: Text.ElideRight
                label2.text: i18n("Set the nighttime perceived brightness percentage.")
                label2.wrapMode: Text.Wrap

                template.content: SpinBox
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
                    Component.onCompleted: root.prepareResponsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    from: 1
                    to: 200
                    stepSize: 5
                    value: root.hyprsunset ? root.hyprsunset.nighttimeGammaPercent : 80
                    onValueModified: if (root.hyprsunset) root.hyprsunset.nighttimeGammaPercent = value
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
            if (root.editingDaytimeProfile)
            {
                root.tempHour = root.hyprsunset ? root.hyprsunset.daytimeHour : 7
                root.tempMinute = root.hyprsunset ? root.hyprsunset.daytimeMinute : 0
            }
            else
            {
                root.tempHour = root.hyprsunset ? root.hyprsunset.nighttimeHour : 19
                root.tempMinute = root.hyprsunset ? root.hyprsunset.nighttimeMinute : 0
            }
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
                    if (root.hyprsunset)
                    {
                        if (root.editingDaytimeProfile)
                        {
                            root.hyprsunset.daytimeHour = root.tempHour
                            root.hyprsunset.daytimeMinute = root.tempMinute
                        }
                        else
                        {
                            root.hyprsunset.nighttimeHour = root.tempHour
                            root.hyprsunset.nighttimeMinute = root.tempMinute
                        }
                    }
                    timeDialog.close()
                }
            }
        ]

        RowLayout
        {
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

    Component.onCompleted: reloadSettings()
}
