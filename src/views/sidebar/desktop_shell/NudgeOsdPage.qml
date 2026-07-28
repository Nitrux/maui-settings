import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var info: (typeof nudgeOsdInfo !== "undefined" && nudgeOsdInfo) ? nudgeOsdInfo : null

    property var iconModeLabels: [i18n("System icons"), i18n("Emoji icons")]
    property var iconModeValues: ["system", "emoji"]

    function indexForValue(model, value)
    {
        for (let i = 0; i < model.length; ++i)
        {
            if (model[i] === value)
                return i
        }
        return 0
    }

    function reloadSettings()
    {
        if (info)
            info.reload()
    }

    function saveSettings()
    {
        return info ? info.save() : false
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("NudgeOSD Settings")
        text2: i18n("Configure the on-screen display.")
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _appearanceLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _appearanceLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Appearance")
            description: i18n("Configure the icon source and OSD dimensions.")
            template.label2.wrapMode: Text.Wrap

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Icon style")
                label1.elide: Text.ElideRight
                label2.text: i18n("Use icon-theme artwork or Nerd Font symbols.")
                label2.wrapMode: Text.Wrap
                template.content: ComboBox
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: root.iconModeLabels
                    currentIndex: info ? root.indexForValue(root.iconModeValues, info.iconMode) : 0
                    onActivated: if (info) info.iconMode = root.iconModeValues[currentIndex]
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Width")
                label1.elide: Text.ElideRight
                label2.text: i18n("OSD width in pixels.")
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
                    from: 160
                    to: 800
                    value: info ? info.osdWidth : 292
                    onValueModified: if (info) info.osdWidth = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Height")
                label1.elide: Text.ElideRight
                label2.text: i18n("OSD height in pixels.")
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
                    from: 56
                    to: 200
                    value: info ? info.osdHeight : 66
                    onValueModified: if (info) info.osdHeight = value
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
        implicitHeight: _positionLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _positionLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Position")
            description: i18n("Configure the OSD position on the active screen.")
            template.label2.wrapMode: Text.Wrap

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Bottom offset")
                label1.elide: Text.ElideRight
                label2.text: i18n("Distance from the bottom edge in pixels.")
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
                    from: 0
                    to: 1000
                    value: info ? info.bottomOffset : 114
                    onValueModified: if (info) info.bottomOffset = value
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
        implicitHeight: _behaviorLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _behaviorLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Behavior")
            description: i18n("Configure visibility and animation timing.")
            template.label2.wrapMode: Text.Wrap

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Display duration")
                label1.elide: Text.ElideRight
                label2.text: i18n("Milliseconds before the OSD begins to hide.")
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
                    from: 250
                    to: 10000
                    stepSize: 50
                    value: info ? info.hideTimeout : 2000
                    onValueModified: if (info) info.hideTimeout = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Show animation")
                label1.elide: Text.ElideRight
                label2.text: i18n("Fade-in duration in milliseconds.")
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
                    from: 0
                    to: 2000
                    stepSize: 25
                    value: info ? info.showAnimationDuration : 200
                    onValueModified: if (info) info.showAnimationDuration = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Hide animation")
                label1.elide: Text.ElideRight
                label2.text: i18n("Fade-out duration in milliseconds.")
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
                    from: 0
                    to: 2000
                    stepSize: 25
                    value: info ? info.hideAnimationDuration : 200
                    onValueModified: if (info) info.hideAnimationDuration = value
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
        implicitHeight: _controlsLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _controlsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Controls")
            description: i18n("Set the default adjustment made by NudgeOSD commands.")
            template.label2.wrapMode: Text.Wrap

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Volume step")
                label1.elide: Text.ElideRight
                label2.text: i18n("Default percentage changed by volume shortcuts.")
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
                    from: 1
                    to: 100
                    value: info ? info.volumeStep : 5
                    onValueModified: if (info) info.volumeStep = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Brightness step")
                label1.elide: Text.ElideRight
                label2.text: i18n("Default percentage changed by brightness shortcuts.")
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
                    from: 1
                    to: 100
                    value: info ? info.brightnessStep : 10
                    onValueModified: if (info) info.brightnessStep = value
                }
            }
        }
    }
}
