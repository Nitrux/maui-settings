# Copyright 2018-2021 Camilo Higuita <milo.h@aol.com>
# Copyright 2018-2021 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: GPL-3.0-or-later

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR})

set(app_SRCS
    main.cpp
    src/modulesmanager.cpp
    src/modulesmodel.cpp
    src/moduleitem.cpp
    src/server/server.cpp)

set(app_RSCRS
    data/data.qrc
    assets/assets.qrc)

set(modulesSRCS
    src/modules/background/backgroundmodule.cpp
    src/modules/background/background.cpp

    src/modules/theme/thememodule.cpp
    src/modules/theme/theme.cpp
    src/modules/theme/iconsmodel.cpp

    src/modules/about/aboutmodule.cpp
    src/modules/about/aboutdata.cpp

    src/modules/screen/screenmodule.cpp
    src/modules/screen/screen.cpp

    src/modules/network/networkmodule.cpp
    src/modules/bluetooth/bluetoothmodule.cpp
    src/modules/audio/audiomodule.cpp
    src/modules/shell/shellmodule.cpp
    src/modules/formfactor/formfactormodule.cpp
    src/modules/accessibility/accessibilitymodule.cpp
    src/modules/screenshot/screenshotmodule.cpp
    src/modules/screenshot/screenshothandler.cpp
    src/modules/inputdevices/inputdevicesmodule.cpp)

set(modulesQML
    main.qml
    src/controls/HomeView.qml
    src/modules/theme/ThemeModuleView.qml
    src/modules/about/AboutModuleView.qml

    src/modules/audio/AudioModuleView.qml
    src/modules/audio/AudioCard.qml
    src/modules/audio/SliderTemplate.qml

    src/modules/background/BackgroundModuleView.qml
    src/modules/screen/ScreenModuleView.qml
    src/modules/formfactor/FormFactorModuleView.qml
    src/modules/network/NetworkModuleView.qml
    src/modules/accessibility/AccessibilityModuleView.qml
    src/modules/screenshot/ScreenshotModuleView.qml
    src/modules/inputdevices/InputDevicesModuleView.qml
    src/modules/theme/StyleTypeButton.qml
    src/modules/theme/IconsPage.qml
    src/modules/theme/ColorSchemesPage.qml
    src/modules/theme/WindowControlsPage.qml
    src/modules/theme/FontsPage.qml
    src/modules/theme/UnitsPage.qml
    src/modules/background/WallpapersPage.qml)


qt_generate_dbus_interface(src/server/server.h org.maui.Settings.xml)
qt_add_dbus_adaptor(app_dbus_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.maui.Settings.xml src/server/server.h Server)
qt_add_dbus_interface(app_dbus_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.maui.Settings.xml serverinterface)

qt_add_executable(${PROJECT_NAME}
    ${app_SRCS}
    ${app_RSCRS}

    ${modulesSRCS}
    ${app_dbus_SRCS})

qt6_add_qml_module(${PROJECT_NAME}
    URI ${PROJECT_NAME}Module
    VERSION 1.0
    QML_FILES ${modulesQML})

target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    MauiKit4
    MauiMan4
    MauiSettingsLib
    CaskServer::CaskServerLib
    MauiCore::Audio
    MauiCore::Notifications
    Qt6::DBus
    Qt6::Quick
    Qt6::Qml
    Qt6::Widgets
    KF6::CoreAddons
    KF6::I18n
    KF6::ConfigCore
    KF6::ConfigWidgets
    KF6::IconThemes)

#set(MAUIMAN_MODULES_IMPORT_PATH "fuckery")
#add_definitions(-DMAUIMAN_MODULES_IMPORT_PATH)
#target_compile_definitions(${PROJECT_NAME} PUBLIC MAUIMAN_MODULES_IMPORT_PATH)

install(TARGETS ${PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES assets/mauisettings.svg DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
install(FILES data/org.maui.settings.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES data/org.maui.settings.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES data/org.maui.settings.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
