import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.mauikit.controls as Maui

Maui.ScrollColumn {
    id: root
    anchors.fill: parent
    spacing: Maui.Style.space.big
    readonly property var info: powerInfo
    readonly property bool daemonReady: info && info.daemonAvailable && info.daemonRunning && info.configAvailable
    function reloadSettings() { if (info) info.reload() }
    function saveSettings() { return info ? info.save() : false }
    function responsive(control) {
        control.responsiveSectionItem = control.parent.parent.parent
        control.wideParent = control.parent
        control.updateResponsiveParent()
    }
    Component.onCompleted: reloadSettings()
    Timer { interval: 5000; repeat: true; running: true; onTriggered: root.reloadSettings() }

    Maui.SectionHeader {
        Layout.fillWidth: true
        text1: i18n("Nitrux Performance Tuning")
        text2: i18n("Configure thresholds that affect the system's performance.")
        label2.wrapMode: Text.Wrap
    }

    Rectangle {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: settingsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout {
            id: settingsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            enabled: root.daemonReady
            spacing: Maui.Style.space.small

            Maui.SectionHeader {
                Layout.fillWidth: true
                text1: i18n("NX Power Daemon")
                text2: !info || !info.daemonAvailable ? i18n("nx-powerd is not installed.")
                       : !info.daemonRunning ? i18n("nx-powerd is not running.")
                       : !info.configAvailable ? i18n("The nx-powerd configuration file was not found.")
                       : i18n("Enable automatic profile changes while keeping battery notifications active.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable Automatic mode")
                label1.elide: Text.ElideRight
                label2.text: i18n("Turn off to select power profiles manually.")
                label2.wrapMode: Text.Wrap
                template.content: Switch {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : implicitWidth
                    checked: info ? info.enabled : true
                    onToggled: if (info) info.enabled = checked
                }
            }

            Maui.SectionItem {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Power saver at or below")
                label1.elide: Text.ElideRight
                label2.text: i18n("Battery percentage at which power saver is selected.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    from: 0; to: 98
                    value: info ? info.powerSaverMax : 20
                    onValueModified: if (info) info.powerSaverMax = value
                }
            }

            Maui.SectionItem {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Balanced up to")
                label1.elide: Text.ElideRight
                label2.text: i18n("Battery percentage below which balanced remains selected.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    from: info ? info.powerSaverMax + 1 : 21; to: 99
                    value: info ? info.balancedMax : 59
                    onValueModified: if (info) info.balancedMax = value
                }
            }

            Maui.SectionItem {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Performance at or above")
                label1.elide: Text.ElideRight
                label2.text: i18n("Battery percentage at which performance is selected.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 18
                    from: info ? info.balancedMax + 1 : 60; to: 100
                    value: info ? info.performanceMin : 60
                    onValueModified: if (info) info.performanceMin = value
                }
            }
        }
    }
}