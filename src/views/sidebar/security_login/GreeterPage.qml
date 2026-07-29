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
    readonly property bool saveAvailable: controller ? controller.saveAvailable : false
    readonly property bool editable: saveAvailable && !controller.loading && !controller.saving

    anchors.fill: parent
    title: i18n("Greeter")
    spacing: Maui.Style.space.big

    function reloadSettings()
    {
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

    function pickColorScheme()
    {
        _colorSchemeDialog.currentPath = controller && controller.colorSchemeDirectory
            ? controller.colorSchemeDirectory : FB.FM.homePath()
        _colorSchemeDialog.browser.settings.viewType = FB.FMList.LIST_VIEW
        _colorSchemeDialog.browser.settings.filterType = FB.FMList.NONE
        _colorSchemeDialog.callback = (paths) =>
        {
            if (controller && paths && paths.length)
                controller.colorSchemePath = paths[0]
        }
        _colorSchemeDialog.open()
    }

    function pickAvatar()
    {
        _avatarDialog.currentPath = FB.FM.homePath()
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
        id: _colorSchemeDialog
        singleSelection: true
        searchBar: true
        mode: FB.FileDialog.Modes.Open
        currentPath: controller && controller.colorSchemeDirectory
            ? controller.colorSchemeDirectory : FB.FM.homePath()
    }

    FB.FileDialog
    {
        id: _avatarDialog
        singleSelection: true
        searchBar: true
        mode: FB.FileDialog.Modes.Open
        currentPath: FB.FM.homePath()
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
                text1: i18n("Appearance and Visuals")
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

                template.content: RowLayout
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
                    Layout.minimumWidth: responsiveNarrow ? 0 : Maui.Style.units.gridUnit * 14
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 20
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 18
                    spacing: Maui.Style.space.small

                    TextField
                    {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        text: controller ? controller.wallpaperPath : ""
                        placeholderText: i18n("/path/to/wallpaper.png")
                        onEditingFinished:
                        {
                            if (controller)
                                controller.wallpaperPath = text
                        }
                    }

                    Button
                    {
                        text: i18n("Choose")
                        onClicked: root.pickWallpaper()
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Color scheme")
                label1.elide: Text.ElideRight
                label2.text: controller
                    ? root.displayPath(controller.colorSchemePath, i18n("Use the default color scheme"))
                    : i18n("Use the default color scheme")
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
                    Layout.minimumWidth: responsiveNarrow ? 0 : Maui.Style.units.gridUnit * 14
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 20
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 18
                    spacing: Maui.Style.space.small

                    TextField
                    {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        text: controller ? controller.colorSchemePath : ""
                        placeholderText: i18n("/path/to/theme.colors")
                        onEditingFinished:
                        {
                            if (controller)
                                controller.colorSchemePath = text
                        }
                    }

                    Button
                    {
                        text: i18n("Choose")
                        onClicked: root.pickColorScheme()
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

                template.content: TextField
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
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 14
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                    text: controller ? controller.iconTheme : ""
                    placeholderText: i18n("hicolor")
                    onEditingFinished: if (controller) controller.iconTheme = text
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Font")
                label1.elide: Text.ElideRight
                label2.text: i18n("Font family used throughout the login screen.")
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
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 14
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 12
                    text: controller ? controller.fontFamily : ""
                    placeholderText: i18n("Noto Sans")
                    onEditingFinished: if (controller) controller.fontFamily = text
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Font size")
                label1.elide: Text.ElideRight
                label2.text: i18n("Font size in points.")
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
                    from: 1
                    to: 256
                    editable: true
                    value: controller ? controller.fontSize : 10
                    onValueModified: if (controller) controller.fontSize = value
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Avatar image")
                label1.elide: Text.ElideRight
                label2.text: controller
                    ? root.displayPath(controller.avatarPath, i18n("Use user account avatars"))
                    : i18n("Use user account avatars")
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
                    Layout.minimumWidth: responsiveNarrow ? 0 : Maui.Style.units.gridUnit * 14
                    Layout.maximumWidth: responsiveNarrow
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 20
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 18
                    spacing: Maui.Style.space.small

                    TextField
                    {
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        text: controller ? controller.avatarPath : ""
                        placeholderText: i18n("Automatic")
                        onEditingFinished: if (controller) controller.avatarPath = text
                    }

                    Button
                    {
                        text: i18n("Choose")
                        onClicked: root.pickAvatar()
                    }
                }
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Time format")
                label1.elide: Text.ElideRight
                label2.text: i18n("A Qt date/time format string, for example hh:mm.")
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
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 12
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 10
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
                label2.text: i18n("A Qt date/time format string, for example dddd, d MMMM yyyy.")
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
                    text: controller ? controller.dateFormat : "dddd, d MMMM yyyy"
                    placeholderText: "dddd, d MMMM yyyy"
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
                text2: i18n("Choose the initial Wayland session and user-selection behavior.")
                label2.wrapMode: Text.Wrap
            }

            Maui.SectionItem
            {
                Layout.fillWidth: true
                flat: true
                label1.text: i18n("Default Wayland session")
                label1.elide: Text.ElideRight
                label2.text: i18n("Select a discovered session or enter its exact display name.")
                label2.wrapMode: Text.Wrap

                template.content: ComboBox
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
                        ? Number.POSITIVE_INFINITY : Maui.Style.units.gridUnit * 16
                    Layout.preferredWidth: Maui.Style.units.gridUnit * 14
                    editable: true
                    model: controller ? controller.availableSessions : []
                    currentIndex: controller ? model.indexOf(controller.defaultSession) : -1
                    editText: currentIndex < 0 && controller ? controller.defaultSession : ""
                    onActivated:
                    {
                        if (controller)
                            controller.defaultSession = currentText
                    }
                    onAccepted:
                    {
                        if (controller)
                            controller.defaultSession = editText
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
}
