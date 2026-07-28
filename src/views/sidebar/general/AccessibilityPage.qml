import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var kde: (typeof kdeGlobalsInfo !== "undefined" && kdeGlobalsInfo) ? kdeGlobalsInfo : null
    readonly property var theme: (typeof themeInfo !== "undefined" && themeInfo) ? themeInfo : null
    readonly property bool scrollBarOnLeftAvailable: theme !== null && typeof theme.scrollBarOnLeft === "boolean"

    property bool stagedSingleClick: true
    property bool stagedScrollBarOnLeft: false

    function reloadSettings()
    {
        if (kde)
        {
            kde.reload()
            stagedSingleClick = kde.singleClick
        }

        if (scrollBarOnLeftAvailable)
            stagedScrollBarOnLeft = theme.scrollBarOnLeft
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

    Component.onCompleted: reloadSettings()
}
