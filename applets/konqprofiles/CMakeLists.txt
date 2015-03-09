install(DIRECTORY package/
        DESTINATION ${PLASMA_DATA_INSTALL_DIR}/plasmoids/konqprofiles)

install(FILES package/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-konqprofiles.desktop)
