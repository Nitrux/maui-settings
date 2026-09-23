import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var info: (typeof hyprlandInfo !== "undefined" && hyprlandInfo) ? hyprlandInfo : null
    readonly property int controlWidth: Maui.Style.units.gridUnit * 13
    readonly property int spinBoxWidth: Maui.Style.units.gridUnit * 7
    readonly property bool resetAvailable: root.info !== null && root.info.available && (
        root.info.layout !== "dwindle"
        || root.info.gapsIn !== 4
        || root.info.gapsOut !== 8
        || root.info.activeOpacity !== 100
        || root.info.inactiveOpacity !== 80
        || !root.info.blurEnabled
        || root.info.blurSize !== 6
        || root.info.blurPasses !== 3
        || !root.info.animationsEnabled)

    property var layoutLabels: [i18n("Dwindle"), i18n("Master")]
    property var layoutValues: ["dwindle", "master"]
    property var ruleTypeLabels: [i18n("Window rule"), i18n("Layer rule")]
    property var ruleTypeValues: ["window", "layer"]
    property var windowMatchLabels: [i18n("Class"), i18n("Title"), i18n("App ID")]
    property var windowMatchValues: ["class", "title", "appid"]
    property var layerMatchLabels: [i18n("Namespace")]
    property var layerMatchValues: ["namespace"]
    property var windowActionLabels: [i18n("No action"), i18n("Float"), i18n("Center"), i18n("Stay focused"), i18n("Focus on activate"), i18n("Suppress maximize events")]
    property var windowActionValues: ["", "float", "center", "stay_focused", "focus_on_activate", "suppress_event"]
    property var layerActionLabels: [i18n("No action"), i18n("Blur"), i18n("Blur popups"), i18n("Ignore alpha")]
    property var layerActionValues: ["", "blur", "blur_popups", "ignore_alpha"]
    property int ruleTypeIndex: 0
    property int ruleMatchIndex: 0
    property int ruleActionIndex: 0
    property int pendingRuleIndex: -1
    property int editingRuleIndex: -1

    function indexForValue(model, value)
    {
        for (let i = 0; i < model.length; ++i)
        {
            if (model[i] === value)
                return i
        }
        return 0
    }

    function responsive(control)
    {
        control.responsiveSectionItem = control.parent.parent.parent
        control.wideParent = control.parent
        control.updateResponsiveParent()
    }

    function clearWindowRuleEditor()
    {
        ruleTypeIndex = 0
        ruleMatchIndex = 0
        ruleActionIndex = 0
        editingRuleIndex = -1
        _ruleNameField.clear()
        _ruleMatchValueField.clear()
    }

    function addWindowRule()
    {
        clearWindowRuleEditor()
        _windowRuleDialog.open()
    }

    function editWindowRule(index, rule)
    {
        editingRuleIndex = index
        ruleTypeIndex = indexForValue(ruleTypeValues, rule.type)
        ruleMatchIndex = indexForValue(ruleTypeIndex === 1 ? layerMatchValues : windowMatchValues, rule.matchKey)
        ruleActionIndex = indexForValue(ruleTypeIndex === 1 ? layerActionValues : windowActionValues, rule.action)
        _ruleNameField.text = rule.name || ""
        _ruleMatchValueField.text = rule.matchValue || ""
        _windowRuleDialog.open()
    }

    function applyWindowRule()
    {
        if (!root.info)
            return

        var matchValues = root.ruleTypeIndex === 1 ? root.layerMatchValues : root.windowMatchValues
        var actionValues = root.ruleTypeIndex === 1 ? root.layerActionValues : root.windowActionValues
        var updated = root.editingRuleIndex >= 0
            ? root.info.updateWindowRule(root.editingRuleIndex, _ruleNameField.text,
                                        matchValues[root.ruleMatchIndex], _ruleMatchValueField.text,
                                        actionValues[root.ruleActionIndex])
            : root.info.addWindowRule(root.ruleTypeValues[root.ruleTypeIndex], _ruleNameField.text,
                                      matchValues[root.ruleMatchIndex], _ruleMatchValueField.text,
                                      actionValues[root.ruleActionIndex])
        if (updated)
            _windowRuleDialog.close()
    }

    function removeWindowRule(index)
    {
        root.pendingRuleIndex = index
        _removeWindowRuleDialog.open()
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

    function resetSettings()
    {
        if (!root.info)
            return

        root.info.layout = "dwindle"
        root.info.gapsIn = 4
        root.info.gapsOut = 8
        root.info.activeOpacity = 100
        root.info.inactiveOpacity = 80
        root.info.blurEnabled = true
        root.info.blurSize = 6
        root.info.blurPasses = 3
        root.info.animationsEnabled = true
    }

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Hyprland Settings")
        text2: !root.info || !root.info.available
            ? i18n("The Hyprland Lua configuration is not available.")
            : i18n("Configure the compositor.")
        label2.wrapMode: Text.Wrap
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _layoutSection.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _layoutSection
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Layout")
                text2: i18n("Choose the tiling layout and the space around windows.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Layout style")
                label1.elide: Text.ElideRight
                label2.text: i18n("Use a dynamically split layout or a master window layout.")
                label2.wrapMode: Text.Wrap
                template.content: ComboBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.controlWidth
                    Layout.preferredWidth: root.controlWidth
                    model: root.layoutLabels
                    currentIndex: root.info ? root.indexForValue(root.layoutValues, root.info.layout) : 0
                    onActivated: if (root.info) root.info.layout = root.layoutValues[currentIndex]
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Inner gaps")
                label1.elide: Text.ElideRight
                label2.text: i18n("Space between tiled windows in pixels.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.spinBoxWidth
                    Layout.preferredWidth: root.spinBoxWidth
                    from: 0
                    to: 64
                    value: root.info ? root.info.gapsIn : 4
                    onValueModified: if (root.info) root.info.gapsIn = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Outer gaps")
                label1.elide: Text.ElideRight
                label2.text: i18n("Space between windows and the screen edge in pixels.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.spinBoxWidth
                    Layout.preferredWidth: root.spinBoxWidth
                    from: 0
                    to: 64
                    value: root.info ? root.info.gapsOut : 8
                    onValueModified: if (root.info) root.info.gapsOut = value
                }
            }


        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _opacitySection.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _opacitySection
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Window Opacity")
                text2: i18n("Adjust the opacity used for active and inactive windows.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Active window opacity")
                label1.elide: Text.ElideRight
                label2.text: i18n("Opacity percentage for the focused window.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.spinBoxWidth
                    Layout.preferredWidth: root.spinBoxWidth
                    from: 10
                    to: 100
                    value: root.info ? root.info.activeOpacity : 100
                    onValueModified: if (root.info) root.info.activeOpacity = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Inactive window opacity")
                label1.elide: Text.ElideRight
                label2.text: i18n("Opacity percentage for unfocused windows.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.spinBoxWidth
                    Layout.preferredWidth: root.spinBoxWidth
                    from: 10
                    to: 100
                    value: root.info ? root.info.inactiveOpacity : 80
                    onValueModified: if (root.info) root.info.inactiveOpacity = value
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _blurSection.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _blurSection
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Blur")
                text2: i18n("Control the blur applied behind transparent surfaces.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable blur")
                label1.elide: Text.ElideRight
                label2.text: i18n("Apply blur effects to windows and layer-shell surfaces.")
                label2.wrapMode: Text.Wrap
                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    checked: root.info ? root.info.blurEnabled : true
                    onToggled: if (root.info) root.info.blurEnabled = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: root.info ? root.info.blurEnabled : false
                label1.text: i18n("Blur size")
                label1.elide: Text.ElideRight
                label2.text: i18n("Radius of the blur kernel.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.spinBoxWidth
                    Layout.preferredWidth: root.spinBoxWidth
                    from: 1
                    to: 20
                    value: root.info ? root.info.blurSize : 6
                    onValueModified: if (root.info) root.info.blurSize = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: root.info ? root.info.blurEnabled : false
                label1.text: i18n("Blur passes")
                label1.elide: Text.ElideRight
                label2.text: i18n("Number of blur passes used for the effect.")
                label2.wrapMode: Text.Wrap
                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : root.spinBoxWidth
                    Layout.preferredWidth: root.spinBoxWidth
                    from: 1
                    to: 10
                    value: root.info ? root.info.blurPasses : 3
                    onValueModified: if (root.info) root.info.blurPasses = value
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _animationSection.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _animationSection
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Animations")
                text2: i18n("Enable transitions when windows and workspaces change.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable animations")
                label1.elide: Text.ElideRight
                label2.text: i18n("Animate window, border, fade, and workspace transitions.")
                label2.wrapMode: Text.Wrap
                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem && (Maui.Handy.isMobile || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)
                    function updateResponsiveParent() { if (wideParent && responsiveSectionItem) parent = responsiveNarrow ? responsiveSectionItem.contentItem : wideParent }
                    onResponsiveNarrowChanged: updateResponsiveParent()
                    Component.onCompleted: root.responsive(this)
                    checked: root.info ? root.info.animationsEnabled : true
                    onToggled: if (root.info) root.info.animationsEnabled = checked
                }
            }
        }
    }
    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.info ? root.info.available : false
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _rulesSection.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _rulesSection
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Window Rules")
                text2: i18n("Manage Hyprland window and layer rules.")
                label2.wrapMode: Text.Wrap
            }

            Repeater
            {
                model: root.info ? root.info.windowRules : []

                delegate: Maui.SectionItem
                {
                    required property var modelData
                    required property int index

                    Layout.fillWidth: true
                    flat: true
                    label1.text: modelData.name || i18n("Unnamed rule")
                    label1.elide: Text.ElideRight
                    label1.wrapMode: Text.NoWrap
                    label2.text: (modelData.type === "layer" ? i18n("Layer rule") : i18n("Window rule"))
                        + " · " + modelData.matchKey + " = " + modelData.matchValue
                        + (modelData.details ? " · " + modelData.details : "")
                    label2.elide: Text.ElideRight
                    label2.wrapMode: Text.NoWrap

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
                        Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 8
                        spacing: Maui.Style.space.tiny

                        ToolButton
                        {
                            icon.name: "document-edit"
                            display: ToolButton.IconOnly
                            ToolTip.visible: hovered
                            ToolTip.text: i18n("Edit rule")
                            onClicked: root.editWindowRule(index, modelData)
                        }

                        ToolButton
                        {
                            icon.name: "edit-delete"
                            display: ToolButton.IconOnly
                            ToolTip.visible: hovered
                            ToolTip.text: i18n("Remove rule")
                            onClicked: root.removeWindowRule(index)
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                visible: root.info && root.info.windowRules.length === 0
                flat: true
                label1.text: i18n("No window rules")
                label2.text: i18n("Add a rule with the + button in the header.")
                label2.wrapMode: Text.Wrap
            }
        }
    }

    Maui.SettingsDialog
    {
        id: _removeWindowRuleDialog
        title: i18n("Remove Window Rule")
        persistent: true
        maxWidth: Maui.Style.units.gridUnit * 24

        Label
        {
            Layout.fillWidth: true
            text: i18n("Remove this compositor rule?")
            wrapMode: Text.WordWrap
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: _removeWindowRuleDialog.close()
            },
            Action
            {
                text: i18n("Remove")
                Maui.Controls.status: Maui.Controls.Negative
                onTriggered:
                {
                    if (root.info)
                        root.info.removeWindowRule(root.pendingRuleIndex)
                    _removeWindowRuleDialog.close()
                }
            }
        ]

        onClosed: root.pendingRuleIndex = -1
    }

    Maui.SettingsDialog
    {
        id: _windowRuleDialog
        title: root.editingRuleIndex >= 0 ? i18n("Modify Window Rule") : i18n("Add Window Rule")
        persistent: true

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.small

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Rule type")
                enabled: root.editingRuleIndex < 0
                label2.text: i18n("Choose whether this rule applies to a window or layer.")

                ComboBox
                {
                    Layout.fillWidth: true
                    enabled: root.editingRuleIndex < 0
                    model: root.ruleTypeLabels
                    currentIndex: root.ruleTypeIndex
                    onActivated:
                    {
                        root.ruleTypeIndex = currentIndex
                        root.ruleMatchIndex = 0
                        root.ruleActionIndex = 0
                    }
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Name")
                label2.text: i18n("A descriptive name for this rule.")

                TextField
                {
                    id: _ruleNameField
                    Layout.fillWidth: true
                    placeholderText: i18n("my-window-rule")
                    onAccepted: _ruleMatchValueField.forceActiveFocus()
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Match property")
                label2.text: i18n("Select the property used to identify the surface.")

                ComboBox
                {
                    Layout.fillWidth: true
                    model: root.ruleTypeIndex === 1 ? root.layerMatchLabels : root.windowMatchLabels
                    currentIndex: root.ruleMatchIndex
                    onActivated: root.ruleMatchIndex = currentIndex
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Match value")
                label2.text: i18n("Enter the regular expression or namespace to match.")

                TextField
                {
                    id: _ruleMatchValueField
                    Layout.fillWidth: true
                    placeholderText: i18n("org.example.Application")
                    onAccepted: root.applyWindowRule()
                }
            }

            Maui.FlexSectionItem
            {
                Layout.fillWidth: true
                label1.text: i18n("Action")
                label2.text: i18n("Choose the compositor behavior for matching surfaces.")

                ComboBox
                {
                    Layout.fillWidth: true
                    model: root.ruleTypeIndex === 1 ? root.layerActionLabels : root.windowActionLabels
                    currentIndex: root.ruleActionIndex
                    onActivated: root.ruleActionIndex = currentIndex
                }
            }
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: _windowRuleDialog.close()
            },
            Action
            {
                text: root.editingRuleIndex >= 0 ? i18n("Apply") : i18n("Add")
                enabled: root.info && _ruleNameField.text.trim().length > 0
                    && _ruleMatchValueField.text.trim().length > 0
                onTriggered: root.applyWindowRule()
            }
        ]

        onClosed: root.clearWindowRuleEditor()
    }

}
