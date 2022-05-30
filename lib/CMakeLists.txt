# Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
# Copyright 2018-2020 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

set(KF5_VERSION "5.70.0")
set(REQUIRED_QT_VERSION 5.15.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(AUTOMOC_MOC_OPTIONS -Muri=org.maui.settings)

project(MauiSettingsLib VERSION ${MAUISETTINGS_VERSION})

find_package(ECM ${KF5_VERSION} NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(GNUInstallDirs)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(FeatureSummary)
include(ECMPoQmTools)
include(ECMSetupVersion)
include(ECMGenerateQmlTypes)
include(CMakePackageConfigHelpers)
include(KDEPackageAppTemplates)

##CMAKE PARTS
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/MauiSettingsLib")
option(QUICK_COMPILER "Use QtQuick compiler to improve performance" TRUE)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/MauiSettingsLibConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiSettingsLibConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS  KF5_INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
    )

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MauiSettingsLibConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiSettingsLibConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
    )

install(EXPORT MauiSettingsLibTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE MauiSettingsLibTargets.cmake
    NAMESPACE MauiSettingsLib::)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX MauiSettingsLib
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mauisettingslib_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/MauiSettingsLibConfigVersion.cmake"
    SOVERSION ${PROJECT_VERSION_MAJOR}
    )

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS DBus Core Qml)

if(QUICK_COMPILER)
        find_package(Qt5QuickCompiler)
        set_package_properties(Qt5QuickCompiler PROPERTIES
                DESCRIPTION "Compile QML at build time"
                TYPE OPTIONAL
        )
endif()


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

set(mauisettingslib_HDRS
    code/abstractmodule.h
    )

set(mauisettingslib_SRCS
    code/abstractmodule.cpp
    code/settings_plugin.cpp
    )

if(QUICK_COMPILER)
        qtquick_compiler_add_resources(mauisettingslib_QML_QRC resources.qrc)
else()
        qt5_add_resources(mauisettingslib_QML_QRC resources.qrc)
endif()

add_library(${PROJECT_NAME}
    SHARED
    ${mauisettingslib_SRCS}
    ${mauisettingslib_HDRS}
    ${mauisettingslib_QML_QRC}
    )

if(QUICK_COMPILER)
    add_definitions(-DQUICK_COMPILER)
    target_compile_definitions(${PROJECT_NAME} PUBLIC QUICK_COMPILER)
endif()

#add_library(MauiMan::MauiMan ALIAS MauiMan)

generate_export_header(${PROJECT_NAME} BASE_NAME MauiSettingsLib)

target_include_directories(${PROJECT_NAME}
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/MauiSettingsLib>")

target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt5::Core
    Qt5::DBus
    Qt5::Qml
    )

install(TARGETS ${PROJECT_NAME} EXPORT MauiSettingsLibTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${mauisettingslib_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/mauisettingslib_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/MauiSettingsLib
    COMPONENT Devel)

add_custom_target(mauisettingslib_copy ALL
    COMMAND ${CMAKE_COMMAND} -E
    make_directory ${CMAKE_BINARY_DIR}/bin/org/maui/settings/lib
    COMMAND ${CMAKE_COMMAND} -E
    copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/controls ${CMAKE_BINARY_DIR}/bin/org/maui/settings/lib
    COMMAND ${CMAKE_COMMAND} -E
    copy $<TARGET_FILE:MauiSettingsLib> ${CMAKE_BINARY_DIR}/bin/org/maui/settings/lib
    )

if(QUICK_COMPILER)
    install(FILES controls/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/maui/settings/lib)
else()
    install(DIRECTORY controls/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/maui/settings/lib)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${KDE_INSTALL_QMLDIR}/org/maui/settings/lib)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
