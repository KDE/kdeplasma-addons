project(plasma-notes)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
    )

set(notes_SRCS notes.cpp)
kde4_add_ui_files(notes_SRCS config.ui)

kde4_add_plugin(plasma_applet_notes ${notes_SRCS})
target_link_libraries(plasma_applet_notes
            ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS}
            )
install(
    TARGETS plasma_applet_notes
    DESTINATION ${PLUGIN_INSTALL_DIR}
    )

install(
    FILES plasma-notes-default.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
    )

install(
    FILES notes.svgz
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
