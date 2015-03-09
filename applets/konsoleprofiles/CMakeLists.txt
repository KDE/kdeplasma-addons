install(DIRECTORY package/
        DESTINATION ${PLASMA_DATA_INSTALL_DIR}/plasmoids/konsoleprofiles)

install(FILES package/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-konsoleprofiles.desktop)
