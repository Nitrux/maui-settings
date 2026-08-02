import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.mauikit.filebrowsing as FB

Maui.SettingsPage
{
    id: root

    readonly property var controller: (typeof qmlGreetController !== "undefined"
                                       && qmlGreetController) ? qmlGreetController : null
    readonly property var kde: (typeof kdeGlobalsInfo !== "undefined"
                                && kdeGlobalsInfo) ? kdeGlobalsInfo : null
    readonly property bool saveAvailable: controller ? controller.saveAvailable : false
    readonly property bool editable: saveAvailable && !controller.loading && !controller.saving

    anchors.fill: parent
    title: i18n("Greeter")
    spacing: Maui.Style.space.big

    function reloadSettings()
    {
        if (kde)
            kde.reload()

        if (controller)
            controller.reload()
    }

    function saveSettings()
    {
        return saveAvailable ? controller.save() : false
    }

    function displayPath(path, emptyText)
    {
        if (!path || !path.length)
            return emptyText

        return path.replace(/^file:\/\//, "")
    }

    function indexForString(model, value)
    {
        const needle = (value || "").trim()
        for (let i = 0; i < model.length; ++i)
        {
            if (String(model[i]).trim() === needle)
                return i
        }

        return -1
    }

    function colorSchemeName(path)
    {
        const fileName = root.displayPath(path, "").split("/").pop()
        return fileName.replace(/\.colors$/i, "")
    }

    function fontLabel()
    {
        if (!controller)
            return ""

        const font = Qt.font({
            family: controller.fontFamily,
            pointSize: controller.fontSize
        })
        return kde ? kde.fontLabel(kde.fontToString(font))
                   : i18n("%1, %2", controller.fontFamily, controller.fontSize)
    }

    function openFontDialog()
    {
        if (!controller)
            return

        _fontPicker.mfont = Qt.font({
            family: controller.fontFamily,
            pointSize: controller.fontSize
        })
        _fontDialog.open()
    }

    function pickWallpaper()
    {
        _wallpaperDialog.currentPath = controller && controller.wallpaperDirectory
            ? controller.wallpaperDirectory : FB.FM.homePath()
        _wallpaperDialog.browser.settings.viewType = FB.FMList.ICON_VIEW
        _wallpaperDialog.browser.settings.filterType = FB.FMList.IMAGE
        _wallpaperDialog.callback = (paths) =>
        {
            if (controller && paths && paths.length)
                controller.wallpaperPath = paths[0]
        }
        _wallpaperDialog.open()
    }

    function pickAvatar()
    {
        _avatarDialog.currentPath = controller && controller.avatarDirectory
            ? controller.avatarDirectory : FB.FM.homePath()
        _avatarDialog.browser.settings.viewType = FB.FMList.ICON_VIEW
        _avatarDialog.browser.settings.filterType = FB.FMList.IMAGE
        _avatarDialog.callback = (paths) =>
        {
            if (controller && paths && paths.length)
                controller.avatarPath = paths[0]
        }
        _avatarDialog.open()
    }

    FB.FileDialog
    {
        id: _wallpaperDialog
        singleSelection: true
        searchBar: true
        mode: FB.FileDialog.Modes.Open
        currentPath: controller && controller.wallpaperDirectory
            ? controller.wallpaperDirectory : FB.FM.homePath()
    }

    FB.FileDialog
    {
        id: _avatarDialog
        singleSelection: true
        searchBar: true
        mode: FB.FileDialog.Modes.Open
        currentPath: controller && controller.avatarDirectory
            ? controller.avatarDirectory : FB.FM.homePath()
    }

    Maui.SectionHeader
    {
        Layout.fillWidth: true
        text1: i18n("QMLGreet Settings")
        text2: controller
            ? i18n("Configure the login screen. Settings are read from %1.", controller.configPath)
            : i18n("Configure the login screen.")
        label2.wrapMode: Text.Wrap
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.editable
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _appearanceLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _appearanceLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Appearance")
                text2: i18n("Choose the background, clock formats, theme, and visual effects.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Wallpaper path")
                label1.elide: Text.ElideRight
                label2.text: controller
                    ? root.displayPath(controller.wallpaperPath, i18n("No wallpaper selected"))
                    : i18n("No wallpaper selected")
                label2.wrapMode: Text.Wrap

                template.content: Button
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    text: i18n("Choose")
                    onClicked: root.pickWallpaper()
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Color scheme")
                label1.elide: Text.ElideRight
                label2.text: i18n("Color scheme used by the login screen.")
                label2.wrapMode: Text.Wrap

                template.content: RowLayout
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    spacing: Maui.Style.space.small

                    ComboBox
                    {
                        id: _colorSchemeCombo
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 16
                        model: kde ? kde.colorSchemes : []
                        currentIndex: kde
                            ? root.indexForString(model,
                                root.colorSchemeName(controller ? controller.colorSchemePath : ""))
                            : -1
                        enabled: kde !== null
                        onActivated:
                        {
                            if (controller)
                                controller.colorSchemePath = kde.colorSchemeFilePath(currentText)
                        }
                    }

                    ToolButton
                    {
                        enabled: kde !== null && _colorSchemeCombo.currentIndex >= 0
                        icon.name: "view-preview"
                        onClicked:
                        {
                            _colorSchemePreviewDialog.previewScheme =
                                _colorSchemeCombo.currentText.trim()
                            _colorSchemePreviewDialog.preview =
                                kde.colorSchemePreview(_colorSchemePreviewDialog.previewScheme)
                            _colorSchemePreviewDialog.open()
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
                label2.text: i18n("Icon theme used by the login screen.")
                label2.wrapMode: Text.Wrap

                template.content: RowLayout
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    spacing: Maui.Style.space.small

                    ComboBox
                    {
                        id: _iconThemeCombo
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: Maui.Style.units.gridUnit * 16
                        model: kde ? kde.iconThemes : []
                        currentIndex: kde && controller
                            ? root.indexForString(kde.iconThemeIds, controller.iconTheme) : -1
                        enabled: kde !== null
                        onActivated:
                        {
                            if (controller)
                                controller.iconTheme = kde.iconThemeIds[currentIndex]
                        }
                    }

                    ToolButton
                    {
                        enabled: kde !== null && _iconThemeCombo.currentIndex >= 0
                        icon.name: "view-preview"
                        onClicked:
                        {
                            _iconThemePreviewDialog.previewTheme =
                                kde.iconThemeIds[_iconThemeCombo.currentIndex]
                            _iconThemePreviewDialog.previewName = _iconThemeCombo.currentText
                            _iconThemePreviewDialog.previewIcons =
                                kde.iconThemePreviewIcons(_iconThemePreviewDialog.previewTheme)
                            _iconThemePreviewDialog.open()
                        }
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Font")
                label1.elide: Text.ElideRight
                label2.text: i18n("Font family and size used throughout the login screen.")
                label2.wrapMode: Text.Wrap

                template.content: Button
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    text: root.fontLabel()
                    onClicked: root.openFontDialog()
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Border radius")
                label1.elide: Text.ElideRight
                label2.text: i18n("Corner radius in pixels for cards, fields, buttons, and popup surfaces.")
                label2.wrapMode: Text.Wrap

                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 8
                    from: 0
                    to: 256
                    editable: true
                    value: controller ? controller.borderRadius : 8
                    onValueModified: if (controller) controller.borderRadius = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Time format")
                label1.elide: Text.ElideRight
                label2.text: i18n("Specify the display format for the time.")
                label2.wrapMode: Text.Wrap

                template.content: TextField
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 10
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 8
                    text: controller ? controller.timeFormat : "hh:mm"
                    placeholderText: "hh:mm"
                    onEditingFinished:
                    {
                        if (controller)
                            controller.timeFormat = text
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Date format")
                label1.elide: Text.ElideRight
                label2.text: i18n("Specify the display format for the date.")
                label2.wrapMode: Text.Wrap

                template.content: TextField
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 14
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                    text: controller && controller.dateFormat !== "dddd, dd MMMM yyyy"
                        ? controller.dateFormat : ""
                    placeholderText: "dddd, dd MMMM yyyy"
                    onEditingFinished:
                    {
                        if (controller)
                            controller.dateFormat = text
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Background blur")
                label1.elide: Text.ElideRight
                label2.text: i18n("Blur the selected wallpaper behind the login controls.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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

                    checked: controller ? controller.blurEnabled : true
                    onToggled: if (controller) controller.blurEnabled = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Animations")
                label1.elide: Text.ElideRight
                label2.text: i18n("Enable transitions and visual animations on the login screen.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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

                    checked: controller ? controller.animationsEnabled : true
                    onToggled: if (controller) controller.animationsEnabled = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Background overlay")
                label1.elide: Text.ElideRight
                label2.text: i18n("Draw a theme-colored overlay above the wallpaper.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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

                    checked: controller ? controller.overlayEnabled : true
                    onToggled: if (controller) controller.overlayEnabled = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: controller ? controller.overlayEnabled : true
                label1.text: i18n("Overlay opacity")
                label1.elide: Text.ElideRight
                label2.text: i18n("Opacity of the background overlay as a percentage.")
                label2.wrapMode: Text.Wrap

                template.content: SpinBox
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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
                    Layout.minimumWidth: 0
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 8
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 6
                    from: 0
                    to: 100
                    editable: true
                    value: controller ? Math.round(controller.overlayOpacity * 100) : 76
                    onValueModified:
                    {
                        if (controller)
                            controller.overlayOpacity = value / 100.0
                    }
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.editable
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _sessionLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _sessionLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Session and User Options")
                text2: i18n("Choose the initial Wayland session and configure user presentation.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Default Wayland session")
                label1.elide: Text.ElideRight
                label2.text: i18n("Select a discovered session.")
                label2.wrapMode: Text.Wrap

                template.content: ComboBox
                {
                    Maui.Theme.colorSet: Maui.Theme.Button

                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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
                    Layout.minimumWidth: 0
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 16
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 14
                    model: controller ? controller.availableSessions : []
                    currentIndex: controller ? model.indexOf(controller.defaultSession) : -1
                    onActivated:
                    {
                        if (controller)
                            controller.defaultSession = currentText
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Show user avatars")
                label1.elide: Text.ElideRight
                label2.text: i18n("Display user avatar images in the account list.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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

                    checked: controller ? controller.showAvatars : true
                    onToggled: if (controller) controller.showAvatars = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                enabled: controller ? controller.showAvatars : true
                label1.text: i18n("Avatar image")
                label1.elide: Text.ElideRight
                label2.text: controller && controller.avatarPath.length
                    ? root.displayPath(controller.avatarPath, i18n("Automatic"))
                    : i18n("Automatic")
                label2.wrapMode: Text.Wrap

                template.content: Button
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

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
                    text: i18n("Choose")
                    onClicked: root.pickAvatar()
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Remember last logged-in user")
                label1.elide: Text.ElideRight
                label2.text: i18n("Preselect the user who most recently signed in.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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

                    checked: controller ? controller.rememberLastUser : true
                    onToggled: if (controller) controller.rememberLastUser = checked
                }
            }
        }
    }

    Rectangle
    {
        Layout.fillWidth: true
        enabled: root.editable
        color: Maui.Theme.alternateBackgroundColor
        radius: Maui.Style.radiusV
        border.color: Maui.Theme.backgroundColor
        border.width: 1
        implicitHeight: _indicatorsLayout.implicitHeight + Maui.Style.contentMargins * 2

        ColumnLayout
        {
            id: _indicatorsLayout
            anchors.fill: parent
            anchors.margins: Maui.Style.contentMargins
            spacing: Maui.Style.space.small

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: i18n("Power and System Indicators")
                text2: i18n("Control the information shown alongside the login controls.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Battery indicator")
                label1.elide: Text.ElideRight
                label2.text: i18n("Show battery charge and charging status when a battery is available.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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

                    checked: controller ? controller.showBattery : true
                    onToggled: if (controller) controller.showBattery = checked
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("System resource indicators")
                label1.elide: Text.ElideRight
                label2.text: i18n("Show system resource information on the login screen.")
                label2.wrapMode: Text.Wrap

                template.content: Switch
                {
                    property Item wideParent
                    property Item responsiveSectionItem
                    readonly property bool responsiveNarrow: responsiveSectionItem
                        && (Maui.Handy.isMobile
                            || responsiveSectionItem.width < Maui.Style.units.gridUnit * 30)

                    function updateResponsiveParent()
                    {
                        if (wideParent && responsiveSectionItem)
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

                    checked: controller ? controller.showSystemResources : true
                    onToggled: if (controller) controller.showSystemResources = checked
                }
            }
        }
    }

    Maui.PopupPage
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

    Maui.PopupPage
    {
        id: _colorSchemePreviewDialog
        title: i18n("Color scheme preview")
        persistent: true

        property string previewScheme
        property var preview: ({})

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: Maui.Style.space.medium

            Maui.SectionHeader
            {
                Layout.fillWidth: true
                text1: _colorSchemePreviewDialog.previewScheme
                text2: i18n("A palette sample using the selected scheme.")
            }

            Maui.ColorsRow
            {
                Layout.fillWidth: true
                colors: _colorSchemePreviewDialog.preview.colors || []
            }
        }
    }

    Maui.PopupPage
    {
        id: _fontDialog
        title: i18n("Fonts")
        persistent: true

        Maui.FontPicker
        {
            id: _fontPicker
            Layout.fillWidth: true
            showStyle: false
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
                    if (controller)
                    {
                        const selectedSize = _fontPicker.mfont.pointSize > 0
                            ? _fontPicker.mfont.pointSize : _fontPicker.mfont.pixelSize
                        controller.fontFamily = _fontPicker.mfont.family
                        if (selectedSize > 0)
                            controller.fontSize = selectedSize
                    }

                    _fontDialog.close()
                }
            }
        ]
    }
}
