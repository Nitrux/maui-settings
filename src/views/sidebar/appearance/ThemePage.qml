import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    id: root
    readonly property var theme: (typeof themeInfo !== "undefined" && themeInfo) ? themeInfo : null
    readonly property var kde: (typeof kdeGlobalsInfo !== "undefined" && kdeGlobalsInfo) ? kdeGlobalsInfo : null
    readonly property var gtk: (typeof gtkSettingsInfo !== "undefined" && gtkSettingsInfo) ? gtkSettingsInfo : null

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
    property string stagedWidgetStyle: ""
    property string stagedGtkTheme: ""
    property string stagedGtkIconTheme: ""
    property string stagedGtkCursorTheme: ""
    property string stagedGtkFont: ""
    property int stagedGtkCursorSize: 24
    property string stagedGtkColorScheme: "default"
    property bool stagedGtkEventSounds: true
    property bool stagedGtkInputFeedbackSounds: false
    property string stagedGtkFontHinting: "medium"
    property string stagedGtkFontAntialiasing: "grayscale"
    property string stagedGtkFontRgbaOrder: "rgb"
    property double stagedGtkTextScalingFactor: 1.0
    property string stagedIconTheme: ""
    property string stagedCursorTheme: ""
    property int stagedKdeCursorSize: 24
    property string stagedKdeFontHinting: "slight"
    property string stagedKdeFontAntialiasing: "grayscale"
    property string stagedKdeFontRgbaOrder: "rgb"
    property string stagedColorScheme: ""
    property string stagedDefaultFont: ""
    property string stagedMenuFont: ""
    property string stagedToolBarFont: ""
    property string stagedSmallFont: ""
    property string stagedMonospaceFont: ""
    property string fontDialogSettingName: ""
    property font fontDialogFont

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
            stagedWidgetStyle = kde.widgetStyle
            stagedIconTheme = kde.iconTheme
            stagedCursorTheme = kde.cursorTheme
            stagedKdeCursorSize = kde.cursorSize
            stagedKdeFontHinting = kde.fontHinting
            stagedKdeFontAntialiasing = kde.fontAntialiasing
            stagedKdeFontRgbaOrder = kde.fontRgbaOrder
            stagedColorScheme = kde.colorScheme
            stagedDefaultFont = kde.defaultFont
            stagedMenuFont = kde.menuFont
            stagedToolBarFont = kde.toolBarFont
            stagedSmallFont = kde.smallFont
            stagedMonospaceFont = kde.monospaceFont
        }

        if (gtk)
        {
            gtk.reload()
            stagedGtkTheme = gtk.theme
            stagedGtkIconTheme = gtk.iconTheme
            stagedGtkCursorTheme = gtk.cursorTheme
            stagedGtkFont = gtk.font
            stagedGtkCursorSize = gtk.cursorSize
            stagedGtkColorScheme = gtk.colorScheme
            stagedGtkEventSounds = gtk.eventSounds
            stagedGtkInputFeedbackSounds = gtk.inputFeedbackSounds
            stagedGtkFontHinting = gtk.fontHinting
            stagedGtkFontAntialiasing = gtk.fontAntialiasing
            stagedGtkFontRgbaOrder = gtk.fontRgbaOrder
            stagedGtkTextScalingFactor = gtk.textScalingFactor
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
            kde.widgetStyle = stagedWidgetStyle
            kde.iconTheme = stagedIconTheme
            kde.cursorTheme = stagedCursorTheme
            kde.cursorSize = stagedKdeCursorSize
            kde.fontHinting = stagedKdeFontHinting
            kde.fontAntialiasing = stagedKdeFontAntialiasing
            kde.fontRgbaOrder = stagedKdeFontRgbaOrder
            kde.colorScheme = stagedColorScheme
            kde.defaultFont = stagedDefaultFont
            kde.menuFont = stagedMenuFont
            kde.toolBarFont = stagedToolBarFont
            kde.smallFont = stagedSmallFont
            kde.monospaceFont = stagedMonospaceFont
            kde.save()
        }

        if (gtk)
        {
            gtk.theme = stagedGtkTheme
            gtk.iconTheme = stagedGtkIconTheme
            gtk.cursorTheme = stagedGtkCursorTheme
            gtk.font = stagedGtkFont
            gtk.cursorSize = stagedGtkCursorSize
            gtk.colorScheme = stagedGtkColorScheme
            gtk.eventSounds = stagedGtkEventSounds
            gtk.inputFeedbackSounds = stagedGtkInputFeedbackSounds
            gtk.fontHinting = stagedGtkFontHinting
            gtk.fontAntialiasing = stagedGtkFontAntialiasing
            gtk.fontRgbaOrder = stagedGtkFontRgbaOrder
            gtk.textScalingFactor = stagedGtkTextScalingFactor
            gtk.save()
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

    property var gtkColorSchemes: [i18n("Default"), i18n("Prefer dark"), i18n("Prefer light")]
    property var gtkColorSchemeValues: ["default", "prefer-dark", "prefer-light"]
    property var gtkHintingValues: ["none", "slight", "medium", "full"]
    property var gtkAntialiasingValues: ["none", "grayscale", "rgba"]
    property var gtkRgbaValues: ["none", "rgb", "bgr", "vrgb", "vbgr"]
    property var kdeAntialiasingValues: ["none", "grayscale"]

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

    function indexFor(model, value)
    {
        return indexForString(model, value)
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
        return propertyName === "widgetStyle" ? stagedWidgetStyle : propertyName === "iconTheme" ? stagedIconTheme : propertyName === "colorScheme" ? stagedColorScheme : propertyName === "defaultFont" ? stagedDefaultFont : propertyName === "menuFont" ? stagedMenuFont : propertyName === "toolBarFont" ? stagedToolBarFont : propertyName === "smallFont" ? stagedSmallFont : propertyName === "monospaceFont" ? stagedMonospaceFont : fallback
    }

    function setStagedFont(settingName, font)
    {
        const value = settingName === "gtkFont" && gtk ? gtk.fontToString(font) : kde.fontToString(font)
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
        case "gtkFont":
            stagedGtkFont = value
            break
        }
    }

    function openFontDialog(settingName, value)
    {
        if (!kde)
            return

        fontDialogSettingName = settingName
        fontDialogFont = kde.fontFromString(value || "")
        _fontPickerDialogLoader.active = false
        _fontPickerDialogLoader.active = true
        _fontPickerDialogLoader.item.open()
    }

    Component.onCompleted: reloadSettings()

    anchors.fill: parent
    spacing: Maui.Style.space.big

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("Theme")
        text2: i18n("Configure MauiKit and desktop integration.")
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
                text2: i18n("These values only affect MauiKit apps.")
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
                label1.text: i18n("Widget style")
                label1.elide: Text.ElideRight
                label2.text: i18n("Qt Widgets style used by KDE and other Qt applications.")
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
                        id: _widgetStyleCombo
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 16
                        model: kde ? kde.widgetStyles : []
                        currentIndex: kde ? indexForString(kde.widgetStyleIds, stagedWidgetStyle) : -1
                        enabled: kde !== null
                        onActivated: root.stagedWidgetStyle = kde.widgetStyleIds[currentIndex]
                    }

                    ToolButton
                    {
                        enabled: kde !== null && _widgetStyleCombo.currentIndex >= 0
                        icon.name: "view-preview"
                        onClicked:
                        {
                            _widgetStylePreviewDialog.previewName = _widgetStyleCombo.currentText
                            _widgetStylePreviewDialog.preview = kde.widgetStylePreview(kde.widgetStyleIds[_widgetStyleCombo.currentIndex])
                            _widgetStylePreviewDialog.open()
                        }
                    }
                }
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
            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Cursor size")
                label1.elide: Text.ElideRight
                label2.text: i18n("Pointer size used by KDE and MauiKit applications.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 7
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 7
                    from: 0
                    to: 128
                    value: stagedKdeCursorSize
                    editable: true
                    enabled: kde !== null
                    onValueModified: stagedKdeCursorSize = value
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

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Font hinting")
                label1.elide: Text.ElideRight
                label2.text: i18n("Controls how glyphs are aligned to the pixel grid.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: gtkHintingValues
                    currentIndex: indexFor(gtkHintingValues, stagedKdeFontHinting)
                    enabled: kde !== null
                    onActivated: stagedKdeFontHinting = currentText
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Font antialiasing")
                label1.elide: Text.ElideRight
                label2.text: i18n("Controls how KDE and MauiKit smooth rendered text.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: kdeAntialiasingValues
                    currentIndex: indexFor(kdeAntialiasingValues, stagedKdeFontAntialiasing)
                    enabled: kde !== null
                    onActivated: stagedKdeFontAntialiasing = currentText
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Subpixel order")
                label1.elide: Text.ElideRight
                label2.text: i18n("Controls the RGB order used for subpixel rendering.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: gtkRgbaValues
                    currentIndex: indexFor(gtkRgbaValues, stagedKdeFontRgbaOrder)
                    enabled: kde !== null
                    onActivated: stagedKdeFontRgbaOrder = currentText
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
        implicitHeight: _gtkAppearanceLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _gtkAppearanceLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("GTK Appearance")
                text2: i18n("Configure GTK themes, icons, cursors, fonts, and theme preferences.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("GTK theme")
                label1.elide: Text.ElideRight
                label2.text: i18n("Theme used by GTK applications.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 16
                    spacing: Maui.Style.space.small

                    ComboBox
                    {
                        id: _gtkThemeCombo
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.maximumWidth: parent.responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                        model: gtk ? gtk.themes : []
                        currentIndex: gtk ? indexFor(gtk.themeIds, stagedGtkTheme) : -1
                        enabled: gtk !== null
                        onActivated: root.stagedGtkTheme = gtk.themeIds[currentIndex]
                    }

                    ToolButton
                    {
                        enabled: gtk !== null && _gtkThemeCombo.currentIndex >= 0
                        icon.name: "view-preview"
                        onClicked:
                        {
                            const previews = gtk.gtkThemePreviews(gtk.themeIds[_gtkThemeCombo.currentIndex])
                            _gtkThemePreviewDialog.previewName = _gtkThemeCombo.currentText
                            _gtkThemePreviewDialog.previews = previews
                            _gtkThemePreviewDialog.previewStatus = previews.length > 0
                                ? i18n("Representative GTK widgets rendered with the selected theme.")
                                : i18n("No GTK preview could be rendered. Build the GTK preview helper and ensure the selected toolkit is installed.")
                            _gtkThemePreviewDialog.open()
                        }
                    }
                }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Icon theme")
                label1.elide: Text.ElideRight
                label2.text: i18n("Icons used by GTK applications.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 16
                    spacing: Maui.Style.space.small

                    ComboBox
                    {
                        id: _gtkIconThemeCombo
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.maximumWidth: parent.responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                        model: gtk ? gtk.iconThemes : []
                        currentIndex: gtk ? indexFor(gtk.iconThemeIds, stagedGtkIconTheme) : -1
                        enabled: gtk !== null
                        onActivated: root.stagedGtkIconTheme = gtk.iconThemeIds[currentIndex]
                    }

                    ToolButton
                    {
                        enabled: gtk !== null && kde !== null && _gtkIconThemeCombo.currentIndex >= 0
                        icon.name: "view-preview"
                        onClicked:
                        {
                            _iconThemePreviewDialog.previewTheme = gtk.iconThemeIds[_gtkIconThemeCombo.currentIndex]
                            _iconThemePreviewDialog.previewName = _gtkIconThemeCombo.currentText
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
                label1.text: i18n("Cursor theme")
                label1.elide: Text.ElideRight
                label2.text: i18n("Pointer theme used by GTK applications.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 16
                    spacing: Maui.Style.space.small

                    ComboBox
                    {
                        id: _gtkCursorThemeCombo
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.maximumWidth: parent.responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                        model: gtk ? gtk.cursorThemes : []
                        currentIndex: gtk ? indexFor(gtk.cursorThemeIds, stagedGtkCursorTheme) : -1
                        enabled: gtk !== null
                        onActivated: root.stagedGtkCursorTheme = gtk.cursorThemeIds[currentIndex]
                    }

                    ToolButton
                    {
                        enabled: gtk !== null && kde !== null && _gtkCursorThemeCombo.currentIndex >= 0
                        icon.name: "view-preview"
                        onClicked:
                        {
                            _cursorThemePreviewDialog.previewTheme = gtk.cursorThemeIds[_gtkCursorThemeCombo.currentIndex]
                            _cursorThemePreviewDialog.previewName = _gtkCursorThemeCombo.currentText
                            _cursorThemePreviewDialog.previewImages = kde.cursorThemePreviewImages(_cursorThemePreviewDialog.previewTheme)
                            _cursorThemePreviewDialog.open()
                        }
                    }
                }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Cursor size")
                label1.elide: Text.ElideRight
                label2.text: i18n("Pointer size used by GTK applications.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 7
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 7
                    from: 0
                    to: 128
                    value: stagedGtkCursorSize
                    editable: true
                    enabled: gtk !== null
                    onValueModified: stagedGtkCursorSize = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Font")
                label1.elide: Text.ElideRight
                label2.text: i18n("Font used by GTK applications, for example Noto Sans 10.")
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
                    text: kde ? kde.fontLabel(stagedGtkFont) : stagedGtkFont
                    enabled: gtk !== null && kde !== null
                    onClicked: openFontDialog("gtkFont", stagedGtkFont)
                }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Theme preference")
                label1.elide: Text.ElideRight
                label2.text: i18n("Choose whether GTK applications prefer the default, dark, or light theme.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: gtkColorSchemes
                    currentIndex: indexFor(gtkColorSchemeValues, stagedGtkColorScheme)
                    enabled: gtk !== null
                    onActivated: stagedGtkColorScheme = gtkColorSchemeValues[currentIndex]
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
        implicitHeight: _gtkDetailsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _gtkDetailsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("GTK Details")
                text2: i18n("Configure GTK font rendering, scaling, and sound behavior.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Font hinting")
                label1.elide: Text.ElideRight
                label2.text: i18n("Controls how glyphs are aligned to the pixel grid.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: gtkHintingValues
                    currentIndex: indexFor(gtkHintingValues, stagedGtkFontHinting)
                    enabled: gtk !== null
                    onActivated: stagedGtkFontHinting = currentText
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Font antialiasing")
                label1.elide: Text.ElideRight
                label2.text: i18n("Controls how GTK smooths rendered text.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: gtkAntialiasingValues
                    currentIndex: indexFor(gtkAntialiasingValues, stagedGtkFontAntialiasing)
                    enabled: gtk !== null
                    onActivated: stagedGtkFontAntialiasing = currentText
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Subpixel order")
                label1.elide: Text.ElideRight
                label2.text: i18n("Controls the RGB order used for subpixel rendering.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 13
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 13
                    model: gtkRgbaValues
                    currentIndex: indexFor(gtkRgbaValues, stagedGtkFontRgbaOrder)
                    enabled: gtk !== null
                    onActivated: stagedGtkFontRgbaOrder = currentText
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Text scaling")
                label1.elide: Text.ElideRight
                label2.text: i18n("Scale GTK text from 50% to 300%.")
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
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 8
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 50
                    to: 300
                    value: Math.round(stagedGtkTextScalingFactor * 100)
                    editable: true
                    enabled: gtk !== null
                    onValueModified: stagedGtkTextScalingFactor = value / 100.0
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Event sounds")
                label1.elide: Text.ElideRight
                label2.text: i18n("Enable sounds for GTK interface events.")
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

                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : implicitWidth
                    Layout.preferredWidth: implicitWidth
                    Layout.alignment: Qt.AlignRight
                    checked: stagedGtkEventSounds
                    enabled: gtk !== null
                    onToggled: stagedGtkEventSounds = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Input feedback sounds")
                label1.elide: Text.ElideRight
                label2.text: i18n("Enable sounds for input feedback.")
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

                    Layout.fillWidth: responsiveNarrow
                    Layout.minimumWidth: responsiveNarrow ? 0 : -1
                    Layout.maximumWidth: responsiveNarrow ? Number.POSITIVE_INFINITY : implicitWidth
                    Layout.preferredWidth: implicitWidth
                    Layout.alignment: Qt.AlignRight
                    checked: stagedGtkInputFeedbackSounds
                    enabled: gtk !== null
                    onToggled: stagedGtkInputFeedbackSounds = checked
                }
            }
        }
    }
    Maui.SettingsDialog
    {
        id: _gtkThemePreviewDialog
        title: i18n("GTK Theme Preview")
        persistent: true

        property string previewName
        property string previewStatus: i18n("Representative GTK widgets rendered with the selected theme.")
        property var previews: []

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.medium

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: _gtkThemePreviewDialog.previewName
                text2: _gtkThemePreviewDialog.previewStatus
            }

            Repeater
            {
                model: _gtkThemePreviewDialog.previews

                delegate: ColumnLayout
                {
                    Layout.fillWidth: true
                    spacing: Maui.Style.space.small

                    Maui.SectionHeader
                    {
                        Layout.fillWidth: true
                        text1: modelData.toolkit
                        text2: i18n("Rendered with the selected GTK theme.")
                    }

                    Maui.Icon
                    {
                        readonly property int previewWidth: modelData.width || 460
                        readonly property int previewHeight: modelData.height || 190
                        Layout.alignment: Qt.AlignHCenter
                        Layout.minimumWidth: previewWidth
                        Layout.preferredWidth: previewWidth
                        Layout.maximumWidth: previewWidth
                        Layout.minimumHeight: previewHeight
                        Layout.preferredHeight: previewHeight
                        Layout.maximumHeight: previewHeight
                        source: modelData.image
                        smooth: true
                    }
                }
            }
        }
    }

    Maui.SettingsDialog
    {
        id: _widgetStylePreviewDialog
        title: i18n("Widget Style Preview")
        persistent: true

        property string previewName
        property var preview: ({})

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.medium

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: _widgetStylePreviewDialog.previewName
                text2: i18n("A sample of Qt Widgets rendered with the selected style.")
            }

            Maui.Icon
            {
                readonly property int previewWidth: _widgetStylePreviewDialog.preview.width || 460
                readonly property int previewHeight: _widgetStylePreviewDialog.preview.height || 190
                Layout.alignment: Qt.AlignHCenter
                Layout.minimumWidth: previewWidth
                Layout.preferredWidth: previewWidth
                Layout.maximumWidth: previewWidth
                Layout.minimumHeight: previewHeight
                Layout.preferredHeight: previewHeight
                Layout.maximumHeight: previewHeight
                source: _widgetStylePreviewDialog.preview.image
                smooth: true
            }
        }
    }

    Maui.SettingsDialog
    {
        id: _cursorThemePreviewDialog
        title: i18n("Cursor Theme Preview")
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
                        smooth: false
                    }
                }
            }
        }
    }

    Maui.SettingsDialog
    {
        id: _iconThemePreviewDialog
        title: i18n("Icon Theme Preview")
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
                        smooth: false
                    }
                }
            }
        }
    }

    Maui.SettingsDialog
    {
        id: _colorSchemePreviewDialog
        title: i18n("Color Scheme Preview")
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

    Loader
    {
        id: _fontPickerDialogLoader
        active: false
        sourceComponent: Component
        {
            Maui.FontPickerDialog
            {
                showStyle: false
                mfont: root.fontDialogFont
                model.onlyMonospaced: root.fontDialogSettingName === "monospaceFont"

                onAccepted:
                {
                    if (root.kde && root.fontDialogSettingName.length > 0)
                        root.setStagedFont(root.fontDialogSettingName, mfont)

                    close()
                }
            }
        }
    }

}
