# Copyright 2018-2021 Camilo Higuita <milo.h@aol.com>
# Copyright 2018-2021 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_executable(${PROJECT_NAME}
  main.cpp
  qml.qrc
  src/code/modulesmanager.cpp
  src/code/modulesmodel.cpp
  src/code/moduleitem.cpp
  data/data.qrc

  assets/assets.qrc
)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)


target_link_libraries(${PROJECT_NAME}
    MauiKit
    Qt5::Quick
    Qt5::Qml
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n)

set(MAUIMAN_MODULES_IMPORT_PATH "fuckery")
add_definitions(-DMAUIMAN_MODULES_IMPORT_PATH)
target_compile_definitions(mauimanager PUBLIC MAUIMAN_MODULES_IMPORT_PATH)

install(TARGETS ${PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES assets/nxmanager.svg DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
install(FILES data/org.maui.manager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES data/org.maui.manager.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES data/org.maui.manager.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
