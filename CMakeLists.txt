cmake_minimum_required(VERSION 3.16)

project(maui-settings VERSION 0.1 LANGUAGES CXX)

set(APP_NAME maui-settings)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(CheckCXXCompilerFlag)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Widgets)

find_package(Git QUIET)
if(Git_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    add_definitions(-DGIT_COMMIT_HASH="${GIT_COMMIT_HASH}")
    add_definitions(-DGIT_BRANCH="${GIT_BRANCH}")
else()
    add_definitions(-DGIT_COMMIT_HASH="${PROJECT_VERSION}")
    add_definitions(-DGIT_BRANCH="Stable")
endif()

if(QT_KNOWN_POLICY_QTP0004)
    qt_policy(SET QTP0004 NEW)
endif()

find_package(KF6 REQUIRED COMPONENTS CoreAddons I18n)
find_package(MauiKit4 REQUIRED)

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_URL_CAST_FROM_STRING)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" config_upper)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${CMAKE_BINARY_DIR}/bin")
endforeach()

if(NOT CMAKE_CROSSCOMPILING)
    string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" _host_processor)
    if(_host_processor MATCHES "^(x86_64|amd64)$")
        check_cxx_compiler_flag("-march=x86-64-v3" MAUI_SETTINGS_HAS_X86_64_V3)
        if(MAUI_SETTINGS_HAS_X86_64_V3)
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-march=x86-64-v3>)
        endif()
    endif()
endif()

set(settings_SRCS
    src/main.cpp
)

qt_add_executable(${APP_NAME}
    MANUAL_FINALIZATION
    ${settings_SRCS}
)

qt_add_qml_module(${APP_NAME} URI app.maui.settings
    NO_PLUGIN
    NO_IMPORT_SCAN
    RESOURCE_PREFIX /
    QML_FILES
    src/main.qml
    src/views/Sidebar.qml
    src/views/sidebar/general/GeneralGroup.qml
    src/views/sidebar/general/AboutItem.qml
    src/views/sidebar/appearance/AppearanceGroup.qml
    src/views/sidebar/appearance/BackgroundItem.qml
    src/views/sidebar/appearance/ThemeItem.qml
    src/views/sidebar/desktop_shell/DesktopShellGroup.qml
    src/views/sidebar/desktop_shell/DesktopItem.qml
    src/views/sidebar/desktop_shell/PanelItem.qml
    src/views/sidebar/applications/ApplicationsGroup.qml
    src/views/sidebar/applications/DefaultsItem.qml
    src/views/sidebar/applications/CacheItem.qml
    src/views/sidebar/connectivity/ConnectivityGroup.qml
    src/views/sidebar/connectivity/BluetoothItem.qml
    src/views/sidebar/connectivity/NetworkItem.qml
    src/views/sidebar/hardware_sound/HardwareSoundGroup.qml
    src/views/sidebar/hardware_sound/AudioItem.qml
    src/views/sidebar/hardware_sound/InputDevicesItem.qml
    src/views/sidebar/hardware_sound/FormFactorItem.qml
    src/views/sidebar/hardware_sound/AccessibilityItem.qml
    src/views/sidebar/security_login/SecurityLoginGroup.qml
    src/views/sidebar/security_login/GreeterItem.qml
    src/views/sidebar/security_login/LockScreenItem.qml
)

target_link_libraries(${APP_NAME}
    PRIVATE
    MauiKit4
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::Widgets
    KF6::CoreAddons
    KF6::I18n
)

install(TARGETS ${APP_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE
        @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
endif()

qt_finalize_target(${APP_NAME})
