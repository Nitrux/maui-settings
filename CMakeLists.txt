# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(MauiSettingsManager)

set(MAUISETTINGS_VERSION 1.1.0)

set(REQUIRED_QT_VERSION 6.4)
set(REQUIRED_KF_VERSION 5.204.0)
set(REQUIRED_MAUI_VERSION 4.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(ECM ${REQUIRED_KF_VERSION} NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(GNUInstallDirs)
include(FeatureSummary)
include(CMakePackageConfigHelpers)

include(ECMSetupVersion)
include(ECMGenerateQmlTypes)
include(ECMFindQmlModule)
include(ECMQmlModule)

include(KDEPackageAppTemplates)
include(KDECompilerSettings)
include(KDEInstallDirs)
include(KDECMakeSettings)

add_subdirectory(lib)
add_subdirectory(manager)
add_subdirectory(qpa_theme)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
