project(plasma-ktimemon)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (-DQT3_SUPPORT)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
    )

set(ktimemon_SRCS timemon.cpp sample.cpp )
kde4_add_ui_files(ktimemon_SRCS config.ui)

kde4_add_plugin(plasma_applet_ktimemon ${ktimemon_SRCS})
target_link_libraries(plasma_applet_ktimemon
            ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS}
            )
install(
    TARGETS plasma_applet_ktimemon
    DESTINATION ${PLUGIN_INSTALL_DIR}
    )

install(
    FILES plasma-applet-ktimemon.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
    )

install(FILES
        ktimemon-16.svgz
        ktimemon-22.svgz
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

kde4_install_icons(${ICON_INSTALL_DIR})
