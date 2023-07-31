# Copyright 2018-2021 Camilo Higuita <milo.h@aol.com>
# Copyright 2018-2021 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(MauiSettings VERSION ${MAUISETTINGS_VERSION})

find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED DBus Qml Quick Widgets DBus COMPONENTS)
find_package(KF6 ${REQUIRED_KF_VERSION} REQUIRED I18n CoreAddons IconThemes Config ConfigWidgets COMPONENTS)

find_package(MauiKit4 ${REQUIRED_MAUI_VERSION} REQUIRED)
#find_package(MauiKit4 ${REQUIRED_MAUI_VERSION} REQUIRED COMPONENTS FileBrowsing4)
find_package(MauiKitFileBrowsing4)

find_package(MauiMan4 REQUIRED)
find_package(MauiCore REQUIRED COMPONENTS Audio Notifications Power)
find_package(CaskServer)

include(KDEInstallDirs)
include(KDEClangFormat)

if(${ECM_SOURCE_UNDER_VERSION_CONTROL})
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    add_definitions(-DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}")
    add_definitions(-DGIT_BRANCH="${GIT_BRANCH}")

else()
    add_definitions(-DGIT_COMMIT_HASH="${MAUISETTINGS_VERSION}")
    add_definitions(-DGIT_BRANCH="Stable")
endif()

ecm_setup_version(${MAUISETTINGS_VERSION}
    VARIABLE_PREFIX MAUISETTINGS
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/mauisettings_version.h")

add_subdirectory(app)

ki18n_install(po)

#set(QML_IMPORT_PATH ${CMAKE_SOURCE_DIR}/src/modules ${CMAKE_BINARY_DIR}/src/modules CACHE STRING "" FORCE)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

