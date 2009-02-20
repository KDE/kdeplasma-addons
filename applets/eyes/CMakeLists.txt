project(eyes)

 
set(eyes_SRCS eyes.cpp)

kde4_add_plugin(plasma_applet_eyes ${eyes_SRCS})
target_link_libraries(plasma_applet_eyes
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_eyes
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-eyes.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES eyes.svg
	DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
