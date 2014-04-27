project(nowplaying)

install(DIRECTORY package/
        DESTINATION ${PLASMA_DATA_INSTALL_DIR}/plasmoids/nowplaying)

install(FILES package/metadata.desktop
        DESTINATION ${SERVICES_INSTALL_DIR}
        RENAME plasma-applet-nowplaying.desktop)

