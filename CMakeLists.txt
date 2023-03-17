cmake_minimum_required(VERSION 3.16)

project(MauiSettingsManager)

set(MAUISETTINGS_VERSION 1.1.0)

#include_directories(src/mauiman)

add_subdirectory(lib)
add_subdirectory(manager)
add_subdirectory(qpa_theme)
