project(timer)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/timer)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-timer.desktop)

install(FILES timer.svgz
	DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

