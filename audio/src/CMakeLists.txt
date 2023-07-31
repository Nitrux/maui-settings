#ecm_create_qm_loader(mauicoreaudio_QM_LOADER libcasknotificationsplugin_qt)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/code)

set(USE_GSETTINGS False)
set(USE_GCONF False)

configure_file(code/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

ecm_add_qml_module(${PROJECT_NAME} URI "org.mauicore.audio" VERSION 1.0 CLASSNAME AudioPlugin)

set(audio_SRCS
    code/card.cpp
    code/client.cpp
    code/context.cpp
    code/device.cpp
    code/maps.cpp
    code/operation.cpp
    code/port.cpp
    code/profile.cpp
    code/pulseaudio.cpp
    code/pulseobject.cpp
    code/sink.cpp
    code/sinkinput.cpp
    code/modulemanager.cpp
    code/source.cpp
    code/sourceoutput.cpp
    code/stream.cpp
    code/volumemonitor.cpp
    code/volumeobject.cpp
    code/debug.cpp
    code/server.cpp
    code/streamrestore.cpp
    code/module.cpp
    code/canberracontext.cpp
    code/speakertest.cpp
#    code/qml/listitemmenu.cpp
    code/qml/plugin.cpp
    # qml/microphoneindicator.cpp
    # qml/volumeosd.cpp
    code/qml/volumefeedback.cpp
    code/model/sortfiltermodel.cpp)

ecm_target_qml_sources(${PROJECT_NAME} SOURCES
    controls/PulseObjectFilterModel.qml)

target_sources(${PROJECT_NAME}
    PRIVATE
    ${audio_SRCS})

add_library(MauiCore::Audio ALIAS ${PROJECT_NAME})

generate_export_header(${PROJECT_NAME} BASE_NAME Audio)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME "Audio")

target_include_directories(${PROJECT_NAME}
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/Maui/Core/Audio>")

target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};>")

target_link_libraries(${PROJECT_NAME}
    PRIVATE

    Qt6::Core
    Qt6::Qml
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Quick

    KF6::I18n
    Canberra::Canberra
    PkgConfig::LIBPULSE
    PkgConfig::LIBPULSE_MAINLOOP
    PkgConfig::GIO)

ecm_finalize_qml_module(${PROJECT_NAME} DESTINATION ${KDE_INSTALL_QMLDIR})
ecm_generate_qmltypes(org.mauicore.audio 1.0 DESTINATION ${KDE_INSTALL_QMLDIR}/org/mauicore/audio)

install(TARGETS ${PROJECT_NAME} EXPORT MauiCoreAudioTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    #    ${headers}
    ${CMAKE_CURRENT_BINARY_DIR}/audio_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/Maui/Core/Audio
    COMPONENT Devel)
