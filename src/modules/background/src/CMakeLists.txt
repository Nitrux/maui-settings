# SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
#
# SPDX-License-Identifier: BSD-2-Clause

set(
    sources
    #    code/touchegg_plugin.cpp
    code/backgroundmodule.cpp
    resources.qrc
    )

set(
    pluginData
    controls/qmldir
    )

#kde_source_files_enable_exceptions(${PROJECT_NAME} code/parser.cpp)

add_library(
    ${PROJECT_NAME}
    SHARED
    ${sources} ${pluginData}
    )

target_link_libraries(
    ${PROJECT_NAME}

    PRIVATE
    MauiMan2
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
    )

if (BUILD_SHARED_LIBS)
    add_custom_target(copy_to_bin ALL
        COMMAND ${CMAKE_COMMAND} -E
        make_directory ${CMAKE_BINARY_DIR}/bin/org/mauiman/modules/background
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/controls ${CMAKE_BINARY_DIR}/bin/org/mauiman/modules/background
        COMMAND ${CMAKE_COMMAND} -E
        copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/bin/org/mauiman/modules/background/
        )

    install(DIRECTORY controls/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/mauiman/modules/background)

endif()

install(TARGETS ${PROJECT_NAME} DESTINATION ${KDE_INSTALL_QMLDIR}/org/mauiman/modules/background)
