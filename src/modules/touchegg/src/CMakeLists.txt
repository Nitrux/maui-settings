# SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
#
# SPDX-License-Identifier: BSD-2-Clause

set(
    sources
    code/touchegg_plugin.cpp
code/parser.cpp
code/lists.cpp
    code/memory.cpp
    code/general.h
    resources.qrc
)

set(
    pluginData
    controls/qmldir
)

kde_source_files_enable_exceptions(${PROJECT_NAME} code/parser.cpp)


add_library(
    ToucheggModule
    SHARED
    ${sources} ${pluginData}
)

target_link_libraries(
    ToucheggModule
       
    PRIVATE
        Qt5::Core
        Qt5::Quick
        Qt5::Qml
        Qt5::Xml
)

if (BUILD_SHARED_LIBS)
    add_custom_target(copy_to_bin ALL
        COMMAND ${CMAKE_COMMAND} -E
                make_directory ${CMAKE_BINARY_DIR}/bin/org/nxmanager/touchegg/
        COMMAND ${CMAKE_COMMAND} -E
                copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/controls ${CMAKE_BINARY_DIR}/bin/org/nxmanager/touchegg/
        COMMAND ${CMAKE_COMMAND} -E
                copy $<TARGET_FILE:ToucheggModule> ${CMAKE_BINARY_DIR}/bin/org/nxmanager/touchegg/
    )

    install(DIRECTORY controls/ DESTINATION ${KDE_INSTALL_QMLDIR}/org/nxmanager/touchegg)

endif()

install(TARGETS ToucheggModule DESTINATION ${KDE_INSTALL_QMLDIR}/org/nxmanager/touchegg )
