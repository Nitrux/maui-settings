# SPDX-FileCopyrightText: 2020 Camilo Higuita
#
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

set(KF5_VERSION "5.70.0")
set(REQUIRED_QT_VERSION 5.15.0)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(MauiBackgroundModule LANGUAGES CXX VERSION 1.0.0)

find_package(ECM ${KF5_VERSION} NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

#find_package(MauiMan REQUIRED)
find_package(Qt5 ${REQUIRED_QT_VERSION} COMPONENTS Core Qml Quick REQUIRED)

include(GNUInstallDirs)
include(KDEInstallDirs)
include(KDECMakeSettings)
#include(KDECompilerSettings)
include(FeatureSummary)
include(ECMPoQmTools)
include(ECMSetupVersion)
include(CMakePackageConfigHelpers)
include(KDEPackageAppTemplates)
    
#include_directories(../../man)
add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
