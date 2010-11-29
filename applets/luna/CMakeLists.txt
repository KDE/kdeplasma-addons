project(luna)

set(luna_SRCS luna.cpp)

kde4_add_ui_files(luna_SRCS lunaConfig.ui)
kde4_add_plugin(plasma_applet_luna ${luna_SRCS})
target_link_libraries(plasma_applet_luna
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_luna
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-luna.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES luna.svgz
	DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

kde4_install_icons(${ICON_INSTALL_DIR})
