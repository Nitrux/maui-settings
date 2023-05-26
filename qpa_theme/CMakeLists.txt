project(MauiQPATheme VERSION ${MAUISETTINGS_VERSION})

set(QT_MAJOR_VERSION 5)

set(QT_MIN_VERSION "5.15.2")
set(KF_MIN_VERSION "5.100.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

find_package(ECM ${KF_MIN_VERSION}  REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(GenerateExportHeader)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)


find_package(ECM ${KF_MIN_VERSION}  REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH} )

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED Widgets DBus QuickControls2 WaylandClient)
find_package(Qt${QT_MAJOR_VERSION}Gui ${QT_MIN_VERSION} CONFIG REQUIRED Private)

find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
                Config ConfigWidgets I18n IconThemes KIO Notifications
                WidgetsAddons WindowSystem ConfigWidgets Wayland
)

find_package(MauiMan REQUIRED)

if (QT_MAJOR_VERSION EQUAL "5")
    find_package(QtWaylandScanner ${QT_MIN_VERSION} REQUIRED)
endif()

find_package(Wayland 1.9 REQUIRED Client)

# dependencies for QPA plugin
if (QT_MAJOR_VERSION EQUAL "5")
    find_package(Qt5ThemeSupport REQUIRED)
    set(QT5PLATFORMSUPPORT_LIBS Qt5ThemeSupport::Qt5ThemeSupport)
else()
    set(QT5PLATFORMSUPPORT_LIBS)
endif()
ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.101
)

add_definitions(-DTRANSLATION_DOMAIN=\"maui-qpa-theme\")
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install(po)
endif()
add_subdirectory(src)

#find_package(FontNotoSans)
#set_package_properties(FontNotoSans PROPERTIES
#    PURPOSE "Default sans-serif font -- this is not detected automatically, pass -DCMAKE_DISABLE_FIND_PACKAGE_FontNotoSans=true to mark it ignored."
#    URL "https://www.google.com/get/noto/"
#    TYPE RUNTIME
#)
#find_package(FontHack)
#set_package_properties(FontHack PROPERTIES
#    PURPOSE "Default monospace font -- this is not detected automatically, pass -DCMAKE_DISABLE_FIND_PACKAGE_FontHack=true to mark it ignored."
#    URL "https://sourcefoundry.org/hack/"
#    TYPE RUNTIME
#)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
#kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
