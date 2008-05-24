project(plasma-minimize-all)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PLASMA_INCLUDE_DIR})

set(minimize-all_SRCS
    minimize-all.cpp
)

kde4_add_plugin(plasma_applet_minimize-all ${minimize-all_SRCS})
target_link_libraries(plasma_applet_minimize-all
    ${KDE4_KDEUI_LIBS} ${PLASMA_LIBS} )

install(TARGETS plasma_applet_minimize-all DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-minimize-all.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons(${ICON_INSTALL_DIR})
