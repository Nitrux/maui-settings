# Copyright 2018-2020 Camilo Higuita <milo.h@aol.com>
# Copyright 2018-2020 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

set(MAUIMAN_VERSION 1.0.0)

project(mauiman VERSION ${MAUIMAN_VERSION})

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Qml Core QuickControls2)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(KDEInstallDirs)

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


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

set(mauiman_HDRS
    code/abstractmodule.h
    code/dbusmodulesyncer.h
    )

set(mauiman_SRCS
    code/abstractmodule.cpp
    code/dbusmodulesyncer.cpp
    )

add_library(MauiMan
    SHARED
    ${mauiman_HDRS}
    ${mauiman_SRCS}
    )


generate_export_header(MauiMan BASE_NAME MauiMan)
install(TARGETS MauiMan EXPORT MauiManTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

target_include_directories(MauiMan
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/MauiMan>")

target_include_directories(MauiMan PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")


target_link_libraries(MauiMan
    PUBLIC
    Qt5::Core

    PRIVATE
    Qt5::Qml
    Qt5::QuickControls2
    )


install(FILES
    ${mauiman_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/mauiman_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/MauiMan
    COMPONENT Devel)


add_custom_target(mauiman_copy ALL
    COMMAND ${CMAKE_COMMAND} -E
    make_directory ${CMAKE_BINARY_DIR}/bin/org/mauiman/controls/
    COMMAND ${CMAKE_COMMAND} -E
    copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/controls ${CMAKE_BINARY_DIR}/bin/org/mauiman/controls/
    COMMAND ${CMAKE_COMMAND} -E
    copy $<TARGET_FILE:MauiMan> ${CMAKE_BINARY_DIR}/bin/org/mauiman/controls/
    )

if(QUICK_COMPILER AND NOT ANDROID)
    install(FILES controls/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/mauiman/controls)
else()
    install(DIRECTORY controls/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/mauiman/controls)
endif()

install(TARGETS MauiMan DESTINATION ${KDE_INSTALL_QMLDIR}/org/mauiman/controls)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
