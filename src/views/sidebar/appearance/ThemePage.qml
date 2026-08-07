import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var theme: (typeof themeInfo !== "undefined" && themeInfo) ? themeInfo : null
    readonly property var kde: (typeof kdeGlobalsInfo !== "undefined" && kdeGlobalsInfo) ? kdeGlobalsInfo : null

    property int stagedStyleType: 0
    property string stagedAccentColor: "#26c6da"
    property string stagedWindowControlsTheme: "Nitrux"
    property bool stagedEnableCSD: false
    property bool stagedEnableEffects: false
    property bool stagedAllowCustomStyling: false
    property int stagedBorderRadius: 0
    property int stagedIconSize: 16
    property int stagedPaddingSize: 0
    property int stagedMarginSize: 0
    property int stagedSpacingSize: 0
    property string stagedIconTheme: ""
    property string stagedCursorTheme: ""
    property string stagedColorScheme: ""
    property string stagedDefaultFont: ""
    property string stagedMenuFont: ""
    property string stagedToolBarFont: ""
    property string stagedSmallFont: ""
    property string stagedMonospaceFont: ""

    function reloadSettings()
    {
        if (kde)
            kde.reload()

        if (theme)
        {
            stagedStyleType = theme.styleType
            stagedAccentColor = String(theme.accentColor)
            stagedWindowControlsTheme = theme.windowControlsTheme
            stagedEnableCSD = theme.enableCSD
            stagedEnableEffects = theme.enableEffects
            stagedAllowCustomStyling = theme.allowCustomStyling
            stagedBorderRadius = theme.borderRadius
            stagedIconSize = theme.iconSize
            stagedPaddingSize = theme.paddingSize
            stagedMarginSize = theme.marginSize
            stagedSpacingSize = theme.spacingSize
        }

        if (kde)
        {
            stagedIconTheme = kde.iconTheme
            stagedCursorTheme = kde.cursorTheme
            stagedColorScheme = kde.colorScheme
            stagedDefaultFont = kde.defaultFont
            stagedMenuFont = kde.menuFont
            stagedToolBarFont = kde.toolBarFont
            stagedSmallFont = kde.smallFont
            stagedMonospaceFont = kde.monospaceFont
        }
    }

    function saveSettings()
    {
        if (theme)
        {
            theme.styleType = stagedStyleType
            theme.accentColor = stagedAccentColor
            theme.windowControlsTheme = stagedWindowControlsTheme
            theme.enableCSD = stagedEnableCSD
            theme.enableEffects = stagedEnableEffects
            theme.allowCustomStyling = stagedAllowCustomStyling
            theme.borderRadius = stagedBorderRadius
            theme.iconSize = stagedIconSize
            theme.paddingSize = stagedPaddingSize
            theme.marginSize = stagedMarginSize
            theme.spacingSize = stagedSpacingSize
        }

        if (kde)
        {
            kde.iconTheme = stagedIconTheme
            kde.cursorTheme = stagedCursorTheme
            kde.colorScheme = stagedColorScheme
            kde.defaultFont = stagedDefaultFont
            kde.menuFont = stagedMenuFont
            kde.toolBarFont = stagedToolBarFont
            kde.smallFont = stagedSmallFont
            kde.monospaceFont = stagedMonospaceFont
            kde.save()
        }
    }

    property var styleTypes: [
        i18n("Light"),
        i18n("Dark"),
        i18n("Adaptive"),
        i18n("Auto"),
        i18n("True Black"),
        i18n("Inverted")
    ]

    property var styleTypeValues: [0, 1, 2, 3, 4, 5]

    property var windowControlsThemes: [
        "Arena",
        "Breeze",
        "Cadium",
        "CadiumGloss",
        "Default",
        "Lucid",
        "Nitrux",
        "OSX",
        "Plastico",
        "Windows",
        "Gnome",
        "Android"
    ]

    function indexForValue(model, value)
    {
        for (let i = 0; i < model.length; ++i)
        {
            if (model[i] === value)
                return i
        }

        return 0
    }

    function indexForString(model, value)
    {
        const needle = (value || "").trim()
        for (let i = 0; i < model.length; ++i)
        {
            if (String(model[i]).trim() === needle)
                return i
        }

        return needle.length ? 0 : -1
    }

    function themeStyleType()
    {
        return stagedStyleType
    }

    function themeStyleTypeIndex()
    {
        return indexForValue(styleTypeValues, themeStyleType())
    }

    function themeAccentColor()
    {
        return stagedAccentColor
    }

    function accentColorVisible()
    {
        return themeStyleType() === 0 || themeStyleType() === 1
    }

    function themeWindowControlsTheme()
    {
        return stagedWindowControlsTheme
    }

    function windowControlsThemeModel()
    {
        const current = themeWindowControlsTheme()
        if (!current || windowControlsThemes.indexOf(current) !== -1)
            return windowControlsThemes

        return [current].concat(windowControlsThemes)
    }

    function windowControlsThemeIndex()
    {
        return indexForString(windowControlsThemeModel(), themeWindowControlsTheme())
    }

    function themeEnabled(propertyName)
    {
        return propertyName === "enableCSD" ? stagedEnableCSD : propertyName === "enableEffects" ? stagedEnableEffects : stagedAllowCustomStyling
    }

    function themeNumber(propertyName, fallback)
    {
        return propertyName === "borderRadius" ? stagedBorderRadius : propertyName === "iconSize" ? stagedIconSize : propertyName === "paddingSize" ? stagedPaddingSize : propertyName === "marginSize" ? stagedMarginSize : propertyName === "spacingSize" ? stagedSpacingSize : fallback
    }

    function kdeString(propertyName, fallback)
    {
        return propertyName === "iconTheme" ? stagedIconTheme : propertyName === "colorScheme" ? stagedColorScheme : propertyName === "defaultFont" ? stagedDefaultFont : propertyName === "menuFont" ? stagedMenuFont : propertyName === "toolBarFont" ? stagedToolBarFont : propertyName === "smallFont" ? stagedSmallFont : propertyName === "monospaceFont" ? stagedMonospaceFont : fallback
    }

    function setStagedFont(settingName, font)
    {
        const value = kde.fontToString(font)
        switch (settingName)
        {
        case "defaultFont":
            stagedDefaultFont = value
            break
        case "menuFont":
            stagedMenuFont = value
            break
        case "toolBarFont":
            stagedToolBarFont = value
            break
        case "smallFont":
            stagedSmallFont = value
            break
        case "monospaceFont":
            stagedMonospaceFont = value
            break
        }
    }

    function openFontDialog(settingName, value)
    {
        if (!kde)
            return

        _fontDialog.settingName = settingName
        _fontDialog.pendingFont = kde.fontFromString(value || "")
        _fontPickerLoader.active = false
        _fontPickerLoader.active = true
        _fontDialog.open()
    }

    Component.onCompleted: reloadSettings()

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Theme")
        text2: i18n("Configure MauiKit and desktop appearance settings from forms.")
    }

    Rectangle
    {
        Layout.fillWidth: true
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _mauiThemeLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _mauiThemeLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("MauiKit Theme")
                text2: i18n("These values are written to MauiMan and used by MauiKit components.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Style type")
                label1.elide: Text.ElideRight
                label2.text: i18n("Light, dark, adaptive, auto, true black, or inverted.")
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                    model: styleTypes
                    currentIndex: themeStyleTypeIndex()
                    enabled: theme !== null
                    onActivated:
                    {
                        root.stagedStyleType = styleTypeValues[currentIndex]
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                visible: accentColorVisible()
                label1.text: i18n("MauiKit accent override")
                label1.elide: Text.ElideRight
                label2.text: i18n("Overrides the MauiKit highlight color in Light and Dark modes only.")
                label2.wrapMode: Text.Wrap

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

                    Rectangle
                    {
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 2
                        Layout.preferredHeight: Maui.Style.units.gridUnit * 2
                        radius: width / 2
                        border.color: Maui.Theme.backgroundColor
                        border.width: 1
                        color: themeAccentColor()
                    }

                    TextField
                    {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                        text: themeAccentColor()
                        placeholderText: "#26c6da"
                        enabled: theme !== null
                        onEditingFinished:
                        {
                            root.stagedAccentColor = text.trim()
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Window controls theme")
                label1.elide: Text.ElideRight
                label2.text: i18n("Theme used for MauiKit client-side decoration buttons.")
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                    model: windowControlsThemeModel()
                    currentIndex: windowControlsThemeIndex()
                    enabled: theme !== null
                    onActivated:
                    {
                        root.stagedWindowControlsTheme = currentText.trim()
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable CSD")
                label1.elide: Text.ElideRight
                label2.text: i18n("Use client-side decorations for applications.")
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
                    checked: themeEnabled("enableCSD")
                    enabled: theme !== null
                    onToggled:
                    {
                        root.stagedEnableCSD = checked
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable effects")
                label1.elide: Text.ElideRight
                label2.text: i18n("Allow visual effects such as blur and animations.")
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
                    checked: themeEnabled("enableEffects")
                    enabled: theme !== null
                    onToggled:
                    {
                        root.stagedEnableEffects = checked
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Allow custom styling")
                label1.elide: Text.ElideRight
                label2.text: i18n("Permit third-party Qt Quick Controls styles to opt in.")
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
                    checked: themeEnabled("allowCustomStyling")
                    enabled: theme !== null
                    onToggled:
                    {
                        root.stagedAllowCustomStyling = checked
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
        implicitHeight: _layoutLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _layoutLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Spacing and Shape")
                text2: i18n("These values map to the global Maui UI shape and spacing settings.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Border radius")
                label1.elide: Text.ElideRight
                label2.text: i18n("Corner radius for surfaces and controls.")
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 0
                    to: 64
                    value: themeNumber("borderRadius", 0)
                    enabled: theme !== null
                    onValueModified:
                    {
                        root.stagedBorderRadius = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Icon size")
                label1.elide: Text.ElideRight
                label2.text: i18n("Preferred icon size in controls and menus.")
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 8
                    to: 64
                    value: themeNumber("iconSize", 16)
                    enabled: theme !== null
                    onValueModified:
                    {
                        root.stagedIconSize = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Padding")
                label1.elide: Text.ElideRight
                label2.text: i18n("Internal padding used by controls.")
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 0
                    to: 32
                    value: themeNumber("paddingSize", 0)
                    enabled: theme !== null
                    onValueModified:
                    {
                        root.stagedPaddingSize = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Margin")
                label1.elide: Text.ElideRight
                label2.text: i18n("Margins used around views and sections.")
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 0
                    to: 32
                    value: themeNumber("marginSize", 0)
                    enabled: theme !== null
                    onValueModified:
                    {
                        root.stagedMarginSize = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Spacing")
                label1.elide: Text.ElideRight
                label2.text: i18n("Spacing used between elements in layouts.")
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
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 0
                    to: 32
                    value: themeNumber("spacingSize", 0)
                    enabled: theme !== null
                    onValueModified:
                    {
                        root.stagedSpacingSize = value
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
        implicitHeight: _kdeLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _kdeLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Desktop Integration")
                text2: i18n("These values affect KDE and MauiKit apps.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Icon theme")
                label1.elide: Text.ElideRight
                label2.text: i18n("Preferred desktop icon theme.")
                label2.wrapMode: Text.Wrap

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

                    ComboBox
                    {
                        id: _iconThemeCombo
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 16
                        model: kde ? kde.iconThemes : []
                        currentIndex: kde ? indexForString(kde.iconThemeIds, stagedIconTheme) : -1
                        enabled: kde !== null
                        onActivated:
                        {
                            root.stagedIconTheme = kde.iconThemeIds[currentIndex]
                        }
                    }

                    ToolButton
                    {
                        enabled: kde !== null && _iconThemeCombo.currentIndex >= 0
                        icon.name: "view-preview"
                        onClicked:
                        {
                            _iconThemePreviewDialog.previewTheme = kde.iconThemeIds[_iconThemeCombo.currentIndex]
                            _iconThemePreviewDialog.previewName = _iconThemeCombo.currentText
                            _iconThemePreviewDialog.previewIcons = kde.iconThemePreviewIcons(_iconThemePreviewDialog.previewTheme)
                            _iconThemePreviewDialog.open()
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Color scheme")
                label1.elide: Text.ElideRight
                label2.text: i18n("Current KDE-spec color scheme.")
                label2.wrapMode: Text.Wrap

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

                    ComboBox
                    {
                        id: _colorSchemeCombo
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 16
                        model: kde ? kde.colorSchemes : []
                        currentIndex: kde ? indexForString(kde.colorSchemes, kdeString("colorScheme", "")) : -1
                        enabled: kde !== null
                        onActivated:
                        {
                            root.stagedColorScheme = currentText.trim()
                        }
                    }

                    ToolButton
                    {
                        enabled: kde !== null
                        icon.name: "view-preview"
                        onClicked:
                        {
                            _colorSchemePreviewDialog.previewScheme = _colorSchemeCombo.currentText.trim()
                            _colorSchemePreviewDialog.preview = kde.colorSchemePreview(_colorSchemePreviewDialog.previewScheme)
                            _colorSchemePreviewDialog.open()
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Cursor theme")
                label1.elide: Text.ElideRight
                label2.text: i18n("Preferred desktop pointer theme.")
                label2.wrapMode: Text.Wrap

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

                    ComboBox
                    {
                        id: _cursorThemeCombo
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 16
                        model: kde ? kde.cursorThemes : []
                        currentIndex: kde ? indexForString(kde.cursorThemeIds, stagedCursorTheme) : -1
                        enabled: kde !== null
                        onActivated: root.stagedCursorTheme = kde.cursorThemeIds[currentIndex]
                    }

                    ToolButton
                    {
                        enabled: kde !== null && _cursorThemeCombo.currentIndex >= 0
                        icon.name: "view-preview"
                        onClicked:
                        {
                            _cursorThemePreviewDialog.previewTheme = kde.cursorThemeIds[_cursorThemeCombo.currentIndex]
                            _cursorThemePreviewDialog.previewName = _cursorThemeCombo.currentText
                            _cursorThemePreviewDialog.previewImages = kde.cursorThemePreviewImages(_cursorThemePreviewDialog.previewTheme)
                            _cursorThemePreviewDialog.open()
                        }
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
        implicitHeight: _fontLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _fontLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Fonts")
                text2: i18n("Configure the fonts.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Default font")
                label1.elide: Text.ElideRight
                label2.text: i18n("Primary UI font used by MauiKit and KDE.")
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
                    text: kde ? kde.fontLabel(kdeString("defaultFont", "")) : ""
                    enabled: kde !== null
                    onClicked:
                    {
                        if (kde)
                            openFontDialog("defaultFont", kdeString("defaultFont", ""))
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Menu font")
                label1.elide: Text.ElideRight
                label2.text: i18n("Font used in KDE application menus.")
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
                    text: kde ? kde.fontLabel(kdeString("menuFont", "")) : ""
                    enabled: kde !== null
                    onClicked:
                    {
                        if (kde)
                            openFontDialog("menuFont", kdeString("menuFont", ""))
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Toolbar font")
                label1.elide: Text.ElideRight
                label2.text: i18n("Font used in KDE application toolbars.")
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
                    text: kde ? kde.fontLabel(kdeString("toolBarFont", "")) : ""
                    enabled: kde !== null
                    onClicked:
                    {
                        if (kde)
                            openFontDialog("toolBarFont", kdeString("toolBarFont", ""))
                    }
                }
            }


            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Small font")
                label1.elide: Text.ElideRight
                label2.text: i18n("Font used by KDE for secondary text.")
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
                    text: kde ? kde.fontLabel(kdeString("smallFont", "")) : ""
                    enabled: kde !== null
                    onClicked:
                    {
                        if (kde)
                            openFontDialog("smallFont", kdeString("smallFont", ""))
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Monospaced font")
                label1.elide: Text.ElideRight
                label2.text: i18n("Font used by KDE for code and fixed-width text.")
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
                    text: kde ? kde.fontLabel(kdeString("monospaceFont", "")) : ""
                    enabled: kde !== null
                    onClicked:
                    {
                        if (kde)
                            openFontDialog("monospaceFont", kdeString("monospaceFont", ""))
                    }
                }
            }
        }
    }

    Maui.SettingsDialog
    {
        id: _cursorThemePreviewDialog
        title: i18n("Cursor theme preview")
        persistent: true

        property string previewTheme
        property string previewName
        property var previewImages: []

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.medium

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: _cursorThemePreviewDialog.previewName
                text2: i18n("A sample of pointers rendered from the selected theme.")
            }

            GridLayout
            {
                Layout.alignment: Qt.AlignHCenter
                columns: 3
                columnSpacing: Maui.Style.space.big
                rowSpacing: Maui.Style.space.big

                Repeater
                {
                    model: _cursorThemePreviewDialog.previewImages

                    Maui.IconItem
                    {
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        iconSource: modelData.image
                        iconSizeHint: Math.max(modelData.width, modelData.height)
                        isMask: false
                        smooth: false
                    }
                }
            }
        }
    }

    Maui.SettingsDialog
    {
        id: _iconThemePreviewDialog
        title: i18n("Icon theme preview")
        persistent: true

        property string previewTheme
        property string previewName
        property var previewIcons: []

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.medium

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: _iconThemePreviewDialog.previewName
                text2: i18n("A small sample of icons rendered from the selected theme.")
            }

            GridLayout
            {
                Layout.alignment: Qt.AlignHCenter
                columns: 4
                columnSpacing: Maui.Style.space.small
                rowSpacing: Maui.Style.space.small

                Repeater
                {
                    model: _iconThemePreviewDialog.previewIcons

                    Maui.IconItem
                    {
                        Layout.alignment: Qt.AlignHCenter
                        iconSource: modelData.icon
                        iconSizeHint: modelData.size
                        isMask: false
                        smooth: false
                    }
                }
            }
        }
    }

    Maui.SettingsDialog
    {
        id: _colorSchemePreviewDialog
        title: i18n("Color scheme preview")
        persistent: true

        property string previewScheme
        property var preview: ({})

        function schemeColor(role, fallback)
        {
            return preview && preview[role] ? preview[role] : fallback
        }

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.medium

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: _colorSchemePreviewDialog.previewScheme
                text2: i18n("A palette and interface sample using the selected scheme.")
            }

            Maui.ColorsRow
            {
                Layout.fillWidth: true
                colors: _colorSchemePreviewDialog.preview.colors || []
            }

            Rectangle
            {
                Layout.fillWidth: true
                Layout.preferredWidth: Maui.Style.units.gridUnit * 20
                implicitHeight: _previewWindowLayout.implicitHeight + Maui.Style.contentMargins * 2
                radius: Maui.Style.radiusV
                color: _colorSchemePreviewDialog.schemeColor("windowBackground", Maui.Theme.backgroundColor)
                border.color: _colorSchemePreviewDialog.schemeColor("focusColor", Maui.Theme.highlightColor)
                border.width: 3
                clip: true

                ColumnLayout
                {
                    id: _previewWindowLayout
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: Maui.Style.contentMargins
                    spacing: Maui.Style.space.small

                    Label
                    {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("Window title")
                        color: _colorSchemePreviewDialog.schemeColor("windowForeground", Maui.Theme.textColor)
                    }

                    RowLayout
                    {
                        Layout.fillWidth: true
                        spacing: Maui.Style.space.small

                        Label
                        {
                            Layout.fillWidth: true
                            text: i18n("Window text")
                            color: _colorSchemePreviewDialog.schemeColor("windowForeground", Maui.Theme.textColor)
                        }

                        Button
                        {
                            Layout.preferredWidth: Maui.Style.units.gridUnit * 6
                            text: i18n("Button")
                            Maui.Theme.backgroundColor: _colorSchemePreviewDialog.schemeColor("buttonBackground", "#3a3a4d")
                            Maui.Theme.textColor: _colorSchemePreviewDialog.schemeColor("buttonForeground", "#f2f2f7")
                            Maui.Theme.highlightColor: _colorSchemePreviewDialog.schemeColor("focusColor", "#26c6da")
                            Maui.Theme.highlightedTextColor: _colorSchemePreviewDialog.schemeColor("selectionForeground", "#ffffff")
                        }
                    }

                    Rectangle
                    {
                        Layout.fillWidth: true
                        implicitHeight: _previewViewLayout.implicitHeight + Maui.Style.contentMargins * 2
                        radius: Maui.Style.radiusV / 2
                        color: _colorSchemePreviewDialog.schemeColor("viewBackground", Maui.Theme.alternateBackgroundColor)

                        ColumnLayout
                        {
                            id: _previewViewLayout
                            anchors.fill: parent
                            anchors.margins: Maui.Style.contentMargins
                            spacing: Maui.Style.space.small

                            RowLayout
                            {
                                Layout.fillWidth: true
                                spacing: Maui.Style.space.small

                                Label
                                {
                                    text: i18n("Normal text")
                                    color: _colorSchemePreviewDialog.schemeColor("viewForeground", Maui.Theme.textColor)
                                }

                                Label
                                {
                                    text: i18n("Link")
                                    color: _colorSchemePreviewDialog.schemeColor("linkForeground", Maui.Theme.linkColor)
                                    font.underline: true
                                }

                                Label
                                {
                                    text: i18n("Visited")
                                    color: _colorSchemePreviewDialog.schemeColor("visitedForeground", Maui.Theme.linkColor)
                                    font.underline: true
                                }
                            }

                            Rectangle
                            {
                                Layout.fillWidth: true
                                implicitHeight: _selectedTextRow.implicitHeight + Maui.Style.space.small
                                radius: Maui.Style.radiusV / 2
                                color: _colorSchemePreviewDialog.schemeColor("selectionBackground", Maui.Theme.highlightColor)

                                RowLayout
                                {
                                    id: _selectedTextRow
                                    anchors.fill: parent
                                    anchors.leftMargin: Maui.Style.space.small
                                    anchors.rightMargin: Maui.Style.space.small
                                    spacing: Maui.Style.space.small

                                    Label
                                    {
                                        text: i18n("Selected text")
                                        color: _colorSchemePreviewDialog.schemeColor("selectionForeground", Maui.Theme.highlightedTextColor)
                                    }

                                    Label
                                    {
                                        text: i18n("Link")
                                        color: _colorSchemePreviewDialog.schemeColor("selectionLinkForeground", Maui.Theme.highlightedTextColor)
                                        font.underline: true
                                    }

                                    Label
                                    {
                                        text: i18n("Visited")
                                        color: _colorSchemePreviewDialog.schemeColor("selectionVisitedForeground", Maui.Theme.highlightedTextColor)
                                        font.underline: true
                                    }
                                }
                            }

                            RowLayout
                            {
                                Layout.fillWidth: true
                                spacing: Maui.Style.space.small

                                Label
                                {
                                    text: i18n("Disabled text")
                                    color: _colorSchemePreviewDialog.schemeColor("inactiveForeground", Maui.Theme.disabledTextColor)
                                }

                                Label
                                {
                                    text: i18n("Link")
                                    color: _colorSchemePreviewDialog.schemeColor("inactiveForeground", Maui.Theme.disabledTextColor)
                                    font.underline: true
                                }

                                Item
                                {
                                    Layout.fillWidth: true
                                }

                                ToolButton
                                {
                                    icon.name: "document-open"
                                    display: ToolButton.IconOnly
                                    flat: false
                                    Maui.Theme.backgroundColor: _colorSchemePreviewDialog.schemeColor("buttonBackground", "#3a3a4d")
                                    Maui.Theme.textColor: _colorSchemePreviewDialog.schemeColor("buttonForeground", "#f2f2f7")
                                    Maui.Theme.highlightColor: _colorSchemePreviewDialog.schemeColor("focusColor", "#26c6da")
                                    Maui.Theme.highlightedTextColor: _colorSchemePreviewDialog.schemeColor("selectionForeground", "#ffffff")
                                }

                                ToolButton
                                {
                                    enabled: false
                                    icon.name: "edit-delete"
                                    display: ToolButton.IconOnly
                                    flat: false
                                    Maui.Theme.backgroundColor: _colorSchemePreviewDialog.schemeColor("buttonBackground", "#3a3a4d")
                                    Maui.Theme.textColor: _colorSchemePreviewDialog.schemeColor("inactiveForeground", "#888899")
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Maui.SettingsDialog
    {
        id: _fontDialog
        title: i18n("Fonts")
        persistent: true

        property string settingName
        property font pendingFont

        Loader
        {
            id: _fontPickerLoader
            Layout.fillWidth: true
            active: true
            sourceComponent: _fontPickerComponent
        }

        Component
        {
            id: _fontPickerComponent

            Maui.FontPicker
            {
                Layout.fillWidth: true
                mfont: _fontDialog.pendingFont
                showStyle: false
                model.onlyMonospaced: _fontDialog.settingName === "monospaceFont"
            }
        }

        actions: [
            Action
            {
                text: i18n("Cancel")
                onTriggered: _fontDialog.close()
            },
            Action
            {
                text: i18n("Accept")
                    onTriggered:
                    {
                        if (kde && _fontDialog.settingName.length > 0)
                            root.setStagedFont(_fontDialog.settingName, _fontPickerLoader.item.mfont)

                    _fontDialog.close()
                }
            }
        ]
    }
}
