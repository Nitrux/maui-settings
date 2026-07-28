import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var info: (typeof marinaInfo !== "undefined" && marinaInfo) ? marinaInfo : null

    property var screenPlacementLabels: [i18n("All screens"), i18n("Active screen")]
    property var screenPlacementValues: ["all", "active"]

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

    function automaticSizeText(value)
    {
        return value === 0 ? i18n("Automatic") : value.toString()
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Marina Settings")
        text2: i18n("Configure the desktop dock.")
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
            description: i18n("Configure launcher sizing and spacing from the screen edge.")
            template.label2.wrapMode: Text.Wrap

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Icon size")
                label1.elide: Text.ElideRight
                label2.text: i18n("Launcher icon size in pixels.")
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
                    from: 32
                    to: 96
                    value: info ? info.iconSize : 48
                    onValueModified: if (info) info.iconSize = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Edge margin")
                label1.elide: Text.ElideRight
                label2.text: i18n("Distance between the dock and the screen edge in pixels.")
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
                    to: 48
                    value: info ? info.edgeMargin : 8
                    onValueModified: if (info) info.edgeMargin = value
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
        implicitHeight: _windowLayout.implicitHeight + Maui.Style.contentMargins * 2

        Maui.SectionGroup
        {
            id: _windowLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            padding: 0
            title: i18n("Placement and Size")
            description: i18n("Choose where Marina appears and how its surface is sized.")
            template.label2.wrapMode: Text.Wrap

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Screen placement")
                label1.elide: Text.ElideRight
                label2.text: i18n("Show Marina on every screen or only on the active screen.")
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
                    model: root.screenPlacementLabels
                    currentIndex: info ? root.indexForValue(root.screenPlacementValues, info.screenPlacement) : 0
                    onActivated: if (info) info.screenPlacement = root.screenPlacementValues[currentIndex]
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Dock width")
                label1.elide: Text.ElideRight
                label2.text: i18n("Minimum surface width in pixels, or Automatic for content sizing.")
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
                    to: 4096
                    stepSize: 16
                    value: info ? info.dockWidth : 0
                    textFromValue: (value, locale) => root.automaticSizeText(value)
                    onValueModified: if (info) info.dockWidth = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Dock height")
                label1.elide: Text.ElideRight
                label2.text: i18n("Surface height in pixels, or Automatic to derive it from icon size.")
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
                    to: 256
                    stepSize: 4
                    value: info ? info.dockHeight : 0
                    textFromValue: (value, locale) => root.automaticSizeText(value)
                    onValueModified: if (info) info.dockHeight = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Show above fullscreen windows")
                label1.elide: Text.ElideRight
                label2.text: i18n("Keep Marina visible over fullscreen applications.")
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
                    checked: info ? info.showAboveFullscreen : false
                    onToggled: if (info) info.showAboveFullscreen = checked
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
            description: i18n("Configure when the dock hides.")
            template.label2.wrapMode: Text.Wrap

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Auto-hide")
                label1.elide: Text.ElideRight
                label2.text: i18n("Hide Marina until the pointer reaches its screen edge.")
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
                    checked: info ? info.autoHide : false
                    onToggled: if (info) info.autoHide = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: info ? info.autoHide : false
                label1.text: i18n("Auto-hide delay")
                label1.elide: Text.ElideRight
                label2.text: i18n("Milliseconds before Marina hides.")
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
                    to: 5000
                    stepSize: 50
                    value: info ? info.autoHideDelay : 650
                    onValueModified: if (info) info.autoHideDelay = value
                }
            }
        }
    }
}
