project(bball)
 
set(bball_SRCS bball.cpp)

kde4_add_ui_files(bball_SRCS bballConfig.ui)

kde4_add_plugin(plasma_applet_bball ${bball_SRCS})
	
target_link_libraries(plasma_applet_bball ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PHONON_LIBS} )
 
install(TARGETS plasma_applet_bball
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-bball.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES bball.svgz football.svgz bounce.ogg
	DESTINATION ${DATA_INSTALL_DIR}/bball/)

kde4_install_icons(${ICON_INSTALL_DIR})

