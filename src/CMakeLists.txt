# Copyright 2018-2021 Camilo Higuita <milo.h@aol.com>
# Copyright 2018-2021 Nitrux Latinoamericana S.C.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_executable(${PROJECT_NAME}
  main.cpp
  qml.qrc
  data/data.qrc
  assets/assets.qrc

#  modules/touchegg/parser.cpp
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

install(TARGETS ${PROJECT_NAME} ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES assets/nxmanager.svg DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
install(FILES data/org.nx.manager.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES data/org.nx.manager.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES data/org.nx.manager.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
