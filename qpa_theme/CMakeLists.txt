project(MauiQPATheme VERSION ${MAUISETTINGS_VERSION})

set(KDE_COMPILERSETTINGS_LEVEL "5.82")

include(ECMDeprecationSettings)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets DBus QuickControls2 WaylandClient)
find_package(Qt6Gui ${REQUIRED_QT_VERSION} CONFIG REQUIRED Private)

find_package(KF6 ${REQUIRED_KF_VERSION} REQUIRED COMPONENTS
                Config ConfigWidgets I18n IconThemes KIO Notifications
                WidgetsAddons WindowSystem ConfigWidgets)

find_package(MauiMan4 REQUIRED)

find_package(Wayland 1.9 REQUIRED Client)

ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.101)

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

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
