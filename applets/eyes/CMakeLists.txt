project(eyes)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/eyes)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-eyes.desktop)

install(FILES eyes.svg
        DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

kde4_install_icons(${ICON_INSTALL_DIR})
