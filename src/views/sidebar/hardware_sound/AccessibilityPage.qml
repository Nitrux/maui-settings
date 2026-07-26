import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var kde: (typeof kdeGlobalsInfo !== "undefined" && kdeGlobalsInfo) ? kdeGlobalsInfo : null

    property bool stagedSingleClick: true

    function reloadSettings()
    {
        if (!kde)
            return

        kde.reload()
        stagedSingleClick = kde.singleClick
    }

    function saveSettings()
    {
        if (!kde)
            return false

        kde.singleClick = stagedSingleClick
        return kde.save()
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
                text2: i18n("Choose how files, folders, and other items are activated.")
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Single-click activation")
                label2.text: i18n("Open files and folders with one click instead of a double-click.")
                label2.wrapMode: Text.WordWrap

                template.content: Switch
                {
                    checked: root.stagedSingleClick
                    enabled: root.kde !== null
                    onToggled: root.stagedSingleClick = checked
                }
            }
        }
    }

    Component.onCompleted: reloadSettings()
}
