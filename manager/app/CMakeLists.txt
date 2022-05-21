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
    src/code/modulesmanager.cpp
    src/code/modulesmodel.cpp
    src/code/moduleitem.cpp
    )

set(app_HDRS
    src/code/modulesmanager.h
    src/code/modulesmodel.h
    src/code/moduleitem.h
    )

set(app_RSCRS
     qml.qrc
    data/data.qrc
    assets/assets.qrc
    )

set(modulesSRCS
    modules/background/src/code/backgroundmodule.cpp
    modules/background/src/code/background.cpp

    modules/theme/code/thememodule.cpp
    modules/theme/code/theme.cpp
    )

set(modulesHDRS
    modules/background/src/code/backgroundmodule.h
    modules/background/src/code/background.h

    modules/theme/code/thememodule.h
    modules/theme/code/theme.h
    )

set(modulesRSCRS
    modules/background/src/resources.qrc
    )

add_executable(${PROJECT_NAME}
    ${app_SRCS}
    ${app_HDRS}
    ${app_RSCRS}

    ${modulesSRCS}
    ${modulesHDRS}
    ${modulesRSCRS}
    )

target_compile_definitions(${PROJECT_NAME}
    PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)


target_link_libraries(${PROJECT_NAME}
    PRIVATE
    MauiKit
    MauiMan
    Qt5::DBus
    Qt5::Quick
    Qt5::Qml
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n)

#set(MAUIMAN_MODULES_IMPORT_PATH "fuckery")
#add_definitions(-DMAUIMAN_MODULES_IMPORT_PATH)
#target_compile_definitions(${PROJECT_NAME} PUBLIC MAUIMAN_MODULES_IMPORT_PATH)

install(TARGETS ${PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES assets/nxmanager.svg DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
install(FILES data/org.maui.manager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES data/org.maui.manager.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES data/org.maui.manager.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})