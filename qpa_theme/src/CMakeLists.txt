
#configure_file(config-platformtheme.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-platformtheme.h )

# qdbusmenubar uses them
#remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY)

set(platformtheme_SRCS
    mauiplatformtheme.cpp
    mauiplatformtheme.h
    main.cpp)

add_library(${PROJECT_NAME} MODULE)

target_sources(${PROJECT_NAME} PRIVATE ${platformtheme_SRCS})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt6::GuiPrivate
    Qt6::DBus
    Qt6::QuickControls2
    KF6::ConfigWidgets
    KF6::ConfigCore
    KF6::IconThemes

    KF6::I18n
    KF6::Notifications
    #        XCB::XCB
    KF6::KIOGui
    Qt::WaylandClient
    Qt::GuiPrivate
    Wayland::Client
    MauiMan4::MauiMan4)

set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/platformthemes/")
install(TARGETS ${PROJECT_NAME} DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/platformthemes)

