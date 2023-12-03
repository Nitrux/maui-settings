# Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
# Copyright 2018-2020 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: GPL-3.0-or-later

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR})

#add_library(${PROJECT_NAME} SHARED)
ecm_add_qml_module(${PROJECT_NAME} URI "org.maui.settings.lib" VERSION 1.0 CLASSNAME MauiSettingsPlugin)

set(mauisettingslib_HDRS
    code/abstractmodule.h)

set(mauisettingslib_SRCS
    code/abstractmodule.cpp
    code/settings_plugin.cpp)

ecm_target_qml_sources(${PROJECT_NAME} SOURCES
    controls/SettingsPage.qml
    controls/GraphicButton.qml)

target_sources(${PROJECT_NAME}
    PRIVATE
    ${mauisettingslib_SRCS})

generate_export_header(${PROJECT_NAME} BASE_NAME MauiSettingsLib)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME "MauiSettingsLib")

target_include_directories(${PROJECT_NAME}
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/MauiSettingsLib>")

target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    #    MauiKit4
    Qt6::Core
    Qt6::DBus
    Qt6::Qml)

ecm_finalize_qml_module(${PROJECT_NAME} DESTINATION ${KDE_INSTALL_QMLDIR})
ecm_generate_qmltypes(org.maui.settings.lib 1.0 DESTINATION ${KDE_INSTALL_QMLDIR}/org/maui/settings/lib)

install(TARGETS ${PROJECT_NAME} EXPORT MauiSettingsLibTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${mauisettingslib_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/mauisettingslib_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/MauiSettingsLib
    COMPONENT Devel)

