import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui

Maui.ScrollColumn
{
    readonly property var theme: (typeof themeInfo !== "undefined" && themeInfo) ? themeInfo : null
    readonly property var kde: (typeof kdeGlobalsInfo !== "undefined" && kdeGlobalsInfo) ? kdeGlobalsInfo : null

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
        return theme && theme.styleType !== undefined ? theme.styleType : 0
    }

    function themeStyleTypeIndex()
    {
        return indexForValue(styleTypeValues, themeStyleType())
    }

    function themeAccentColor()
    {
        return theme && theme.accentColor ? theme.accentColor : "#26c6da"
    }

    function accentColorVisible()
    {
        return themeStyleType() === 0 || themeStyleType() === 1
    }

    function themeWindowControlsTheme()
    {
        return theme && theme.windowControlsTheme ? theme.windowControlsTheme : "Nitrux"
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
        return theme && theme[propertyName] !== undefined ? theme[propertyName] : false
    }

    function themeNumber(propertyName, fallback)
    {
        return theme && theme[propertyName] !== undefined ? theme[propertyName] : fallback
    }

    function kdeString(propertyName, fallback)
    {
        return kde && kde[propertyName] !== undefined && kde[propertyName] !== null ? kde[propertyName] : fallback
    }

    function openFontDialog(dialog, value)
    {
        if (!kde)
            return

        dialog.mfont = kde.fontFromString(value || "")
        dialog.open()
    }

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
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Style type")
                label2.text: i18n("Light, dark, adaptive, auto, true black, or inverted.")
                label2.wrapMode: Text.WordWrap

                template.content: ComboBox
                {
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                    model: styleTypes
                    currentIndex: themeStyleTypeIndex()
                    enabled: theme !== null
                    onActivated:
                    {
                        if (theme)
                            theme.styleType = styleTypeValues[currentIndex]
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                visible: accentColorVisible()
                label1.text: i18n("MauiKit accent override")
                label2.text: i18n("Overrides the MauiKit highlight color in Light and Dark modes only.")
                label2.wrapMode: Text.WordWrap

                template.content: RowLayout
                {
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
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                        text: themeAccentColor()
                        placeholderText: "#26c6da"
                        enabled: theme !== null
                        onEditingFinished:
                        {
                            if (theme)
                                theme.accentColor = text.trim()
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Window controls theme")
                label2.text: i18n("Theme used for MauiKit client-side decoration buttons.")
                label2.wrapMode: Text.WordWrap

                template.content: ComboBox
                {
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 14
                    model: windowControlsThemeModel()
                    currentIndex: windowControlsThemeIndex()
                    enabled: theme !== null
                    onActivated:
                    {
                        if (theme)
                            theme.windowControlsTheme = currentText.trim()
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable CSD")
                label2.text: i18n("Use client-side decorations for applications.")
                label2.wrapMode: Text.WordWrap

                template.content: Switch
                {
                    checked: themeEnabled("enableCSD")
                    enabled: theme !== null
                    onToggled:
                    {
                        if (theme)
                            theme.enableCSD = checked
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Enable effects")
                label2.text: i18n("Allow visual effects such as blur and animations.")
                label2.wrapMode: Text.WordWrap

                template.content: Switch
                {
                    checked: themeEnabled("enableEffects")
                    enabled: theme !== null
                    onToggled:
                    {
                        if (theme)
                            theme.enableEffects = checked
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Allow custom styling")
                label2.text: i18n("Permit third-party Qt Quick Controls styles to opt in.")
                label2.wrapMode: Text.WordWrap

                template.content: Switch
                {
                    checked: themeEnabled("allowCustomStyling")
                    enabled: theme !== null
                    onToggled:
                    {
                        if (theme)
                            theme.allowCustomStyling = checked
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
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Border radius")
                label2.text: i18n("Corner radius for surfaces and controls.")
                label2.wrapMode: Text.WordWrap

                template.content: SpinBox
                {
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 0
                    to: 64
                    value: themeNumber("borderRadius", 0)
                    enabled: theme !== null
                    onValueModified:
                    {
                        if (theme)
                            theme.borderRadius = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Icon size")
                label2.text: i18n("Preferred icon size in controls and menus.")
                label2.wrapMode: Text.WordWrap

                template.content: SpinBox
                {
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 8
                    to: 64
                    value: themeNumber("iconSize", 16)
                    enabled: theme !== null
                    onValueModified:
                    {
                        if (theme)
                            theme.iconSize = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Padding")
                label2.text: i18n("Internal padding used by controls.")
                label2.wrapMode: Text.WordWrap

                template.content: SpinBox
                {
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 0
                    to: 32
                    value: themeNumber("paddingSize", 0)
                    enabled: theme !== null
                    onValueModified:
                    {
                        if (theme)
                            theme.paddingSize = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Margin")
                label2.text: i18n("Margins used around views and sections.")
                label2.wrapMode: Text.WordWrap

                template.content: SpinBox
                {
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 0
                    to: 32
                    value: themeNumber("marginSize", 0)
                    enabled: theme !== null
                    onValueModified:
                    {
                        if (theme)
                            theme.marginSize = value
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Spacing")
                label2.text: i18n("Spacing used between elements in layouts.")
                label2.wrapMode: Text.WordWrap

                template.content: SpinBox
                {
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    from: 0
                    to: 32
                    value: themeNumber("spacingSize", 0)
                    enabled: theme !== null
                    onValueModified:
                    {
                        if (theme)
                            theme.spacingSize = value
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
                text2: i18n("These values come from ~/.config/kdeglobals and affect KDE/Qt apps.")
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Icon theme")
                label2.text: i18n("Preferred desktop icon theme.")
                label2.wrapMode: Text.WordWrap

                template.content: RowLayout
                {
                    spacing: Maui.Style.space.small

                    Rectangle
                    {
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 2.25
                        Layout.preferredHeight: Maui.Style.units.gridUnit * 2.25
                        radius: Maui.Style.radiusV / 2
                        color: Maui.Theme.alternateBackgroundColor
                        border.color: Maui.Theme.backgroundColor
                        border.width: 1

                        Maui.Icon
                        {
                            anchors.centerIn: parent
                            width: Maui.Style.units.gridUnit * 1.75
                            height: Maui.Style.units.gridUnit * 1.75
                            source: "preferences-desktop-icons"
                        }
                    }

                    ComboBox
                    {
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 16
                        model: kde ? kde.iconThemes : []
                        currentIndex: kde ? indexForString(kde.iconThemes, kdeString("iconTheme", "")) : -1
                        enabled: kde !== null
                        onActivated:
                        {
                            if (kde)
                                kde.iconTheme = currentText.trim()
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Color scheme")
                label2.text: i18n("Current KDE color scheme.")
                label2.wrapMode: Text.WordWrap

                template.content: RowLayout
                {
                    spacing: Maui.Style.space.small

                    Rectangle
                    {
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 2.25
                        Layout.preferredHeight: Maui.Style.units.gridUnit * 2.25
                        radius: Maui.Style.radiusV / 2
                        color: kde ? kde.colorSchemePreviewColor(kdeString("colorScheme", "")) : "transparent"
                        border.color: Maui.Theme.backgroundColor
                        border.width: 1
                    }

                    ComboBox
                    {
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 16
                        model: kde ? kde.colorSchemes : []
                        currentIndex: kde ? indexForString(kde.colorSchemes, kdeString("colorScheme", "")) : -1
                        enabled: kde !== null
                        onActivated:
                        {
                            if (kde)
                                kde.colorScheme = currentText.trim()
                        }
                    }
                }
            }
            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Default font")
                label2.text: i18n("Primary UI font used by Qt and KDE.")
                label2.wrapMode: Text.WordWrap

                template.content: ToolButton
                {
                    text: kde ? kde.fontLabel(kdeString("defaultFont", "")) : ""
                    enabled: kde !== null
                    onClicked:
                    {
                        if (kde)
                            openFontDialog(_defaultFontDialog, kdeString("defaultFont", ""))
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Small font")
                label2.text: i18n("Font used for secondary text.")
                label2.wrapMode: Text.WordWrap

                template.content: ToolButton
                {
                    text: kde ? kde.fontLabel(kdeString("smallFont", "")) : ""
                    enabled: kde !== null
                    onClicked:
                    {
                        if (kde)
                            openFontDialog(_smallFontDialog, kdeString("smallFont", ""))
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Monospaced font")
                label2.text: i18n("Font used for code and fixed-width text.")
                label2.wrapMode: Text.WordWrap

                template.content: ToolButton
                {
                    text: kde ? kde.fontLabel(kdeString("monospaceFont", "")) : ""
                    enabled: kde !== null
                    onClicked:
                    {
                        if (kde)
                            openFontDialog(_monoFontDialog, kdeString("monospaceFont", ""))
                    }
                }
            }
        }
    }

    Maui.FontPickerDialog
    {
        id: _defaultFontDialog
        onAccepted:
        {
            if (kde)
                kde.defaultFont = kde.fontToString(font)
        }
    }

    Maui.FontPickerDialog
    {
        id: _smallFontDialog
        onAccepted:
        {
            if (kde)
                kde.smallFont = kde.fontToString(font)
        }
    }

    Maui.FontPickerDialog
    {
        id: _monoFontDialog
        onAccepted:
        {
            if (kde)
                kde.monospaceFont = kde.fontToString(font)
        }
    }
}
