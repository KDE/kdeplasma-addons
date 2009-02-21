project(plasma-applet-nowplaying)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set(nowplaying_SRCS
    nowplaying.cpp
    controls.cpp
    albumart.cpp
    infopanel.cpp)

kde4_add_plugin(plasma_applet_nowplaying ${nowplaying_SRCS})
target_link_libraries(plasma_applet_nowplaying  ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_nowplaying DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-nowplaying.desktop DESTINATION ${SERVICES_INSTALL_DIR})
