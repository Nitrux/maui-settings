# Copyright 2018-2021 Camilo Higuita <milo.h@aol.com>
# Copyright 2018-2021 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: GPL-3.0-or-later

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

set(app_SRCS
    main.cpp
    src/modulesmanager.cpp
    src/modulesmodel.cpp
    src/moduleitem.cpp
    src/server/server.cpp
    )

set(app_RSCRS
    qml.qrc
    data/data.qrc
    assets/assets.qrc
    )

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
    )

set(modulesRSCRS
    src/modules/modules.qrc
    )

qt_generate_dbus_interface(src/server/server.h org.maui.Settings.xml)
qt_add_dbus_adaptor(app_dbus_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.maui.Settings.xml src/server/server.h Server)
qt_add_dbus_interface(app_dbus_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.maui.Settings.xml serverinterface)

add_executable(${PROJECT_NAME}
    ${app_SRCS}
    ${app_RSCRS}

    ${modulesSRCS}
    ${modulesRSCRS}

    ${app_dbus_SRCS}
    )

target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    MauiKit
    MauiMan
    MauiSettingsLib
    MauiCore::Audio
    MauiCore::Notifications
    Qt5::DBus
    Qt5::Quick
    Qt5::Qml
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes)

#set(MAUIMAN_MODULES_IMPORT_PATH "fuckery")
#add_definitions(-DMAUIMAN_MODULES_IMPORT_PATH)
#target_compile_definitions(${PROJECT_NAME} PUBLIC MAUIMAN_MODULES_IMPORT_PATH)

install(TARGETS ${PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES assets/nxmanager.svg DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
install(FILES data/org.maui.settings.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES data/org.maui.settings.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES data/org.maui.settings.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
