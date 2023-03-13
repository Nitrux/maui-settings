
#configure_file(config-platformtheme.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-platformtheme.h )

# qdbusmenubar uses them
remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)

set(platformtheme_SRCS
    mauiplatformtheme.cpp
    mauiplatformtheme.h
    main.cpp
    )

add_library(${PROJECT_NAME} MODULE)
if (QT_MAJOR_VERSION EQUAL "5")

else()

endif()

target_sources(${PROJECT_NAME}  PRIVATE ${platformtheme_SRCS})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt::GuiPrivate
    Qt::DBus
    Qt::QuickControls2
    KF${QT_MAJOR_VERSION}::ConfigWidgets
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::IconThemes

    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::Notifications
    #        XCB::XCB
    KF${QT_MAJOR_VERSION}::KIOGui
    ${QT5PLATFORMSUPPORT_LIBS}
    Qt::WaylandClient
    Qt::GuiPrivate
    Wayland::Client
    MauiMan::MauiMan
    )

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/platformthemes/")
install(TARGETS ${PROJECT_NAME}  DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/platformthemes)

