# Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
# Copyright 2018-2020 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

set(KF5_VERSION "5.70.0")
set(REQUIRED_QT_VERSION 5.15.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(AUTOMOC_MOC_OPTIONS -Muri=org.maui.man)

set(MAUIMAN_VERSION 1.0.0)

project(MauiMan VERSION ${MAUIMAN_VERSION})

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
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/MauiMan")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/MauiManConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiManConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS  KF5_INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
    )

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/MauiManConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/MauiManConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
    )

install(EXPORT MauiManTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE MauiManTargets.cmake
    )

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Qml Core QuickControls2)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

set(mauiman_HDRS
    code/abstractmodule.h
    code/dbusmodulesyncer.h
    code/settingsstore.h
    )

set(mauiman_SRCS
    code/abstractmodule.cpp
    code/dbusmodulesyncer.cpp
    code/settingsstore.cpp
    )

add_library(${PROJECT_NAME}
    SHARED
    ${mauiman_SRCS}
    ${mauiman_HDRS}
    )

add_library(MauiMan::MauiMan ALIAS MauiMan)

generate_export_header(${PROJECT_NAME} BASE_NAME MauiMan)

target_include_directories(${PROJECT_NAME}
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/MauiMan>")

target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt5::Core
    Qt5::Qml
    Qt5::QuickControls2
    )

install(TARGETS ${PROJECT_NAME} EXPORT MauiManTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${mauiman_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/mauiman_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/MauiMan
    COMPONENT Devel)

add_custom_target(mauiman_copy ALL
    COMMAND ${CMAKE_COMMAND} -E
    make_directory ${CMAKE_BINARY_DIR}/bin/org/mauiman/
    COMMAND ${CMAKE_COMMAND} -E
    copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/controls ${CMAKE_BINARY_DIR}/bin/org/mauiman/
    COMMAND ${CMAKE_COMMAND} -E
    copy $<TARGET_FILE:MauiMan> ${CMAKE_BINARY_DIR}/bin/org/mauiman/
    )

if(QUICK_COMPILER AND NOT ANDROID)
    install(FILES controls/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/mauiman/controls)
else()
    install(DIRECTORY controls/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/mauiman/controls)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${KDE_INSTALL_QMLDIR}/org/mauiman)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
