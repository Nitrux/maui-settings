# Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
# Copyright 2018-2020 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

set(AUTOMOC_MOC_OPTIONS -Muri=org.maui.settings.lib)

include(ECMGenerateQmlTypes)
include(ECMFindQmlModule)
include(ECMQmlModule)

project(MauiSettingsLib VERSION ${MAUISETTINGS_VERSION})

##CMAKE PARTS
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/MauiSettingsLib")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/MauiSettingsLibConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiSettingsLibConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_PREFIX)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MauiSettingsLibConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiSettingsLibConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel)

install(EXPORT MauiSettingsLibTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE MauiSettingsLibTargets.cmake
    NAMESPACE MauiSettingsLib::)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX MauiSettingsLib
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mauisettingslib_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/MauiSettingsLibConfigVersion.cmake"
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mauisettingslib_version.h"
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/Maui/Settings/Lib
    COMPONENT Devel)

find_package(Qt6 ${REQUIRED_QT_VERSION} COMPONENTS DBus Core Qml REQUIRED)
#find_package(MauiKit4 4 REQUIRED)

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
