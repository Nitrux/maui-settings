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

project(MauiManServer VERSION ${MAUIMAN_VERSION})

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

find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core DBus)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

set(mauimanserver_HDRS
    src/server.h
    src/modules/background.h
    src/modules/theme.h
    )

set(mauimanserver_SRCS
    src/main.cpp
    src/server.cpp
    src/modules/background.cpp
    src/modules/theme.cpp
    )

qt5_add_dbus_adaptor(DBUS_SOURCES src/modules/org.mauiman.Background.xml src/modules/background.h Background)
qt5_add_dbus_adaptor(DBUS_SOURCES src/modules/org.mauiman.Theme.xml src/modules/theme.h Theme)

set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${PROJECT_NAME}

    ${mauimanserver_SRCS}
    ${mauimanserver_HDRS}

    ${DBUS_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/MauiManServer>")

target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    MauiMan
    Qt5::Core
    Qt5::DBus
    )

install(TARGETS ${PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
