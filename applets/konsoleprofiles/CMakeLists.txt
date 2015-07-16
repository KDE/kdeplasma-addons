install(DIRECTORY package/
        DESTINATION ${PLASMA_DATA_INSTALL_DIR}/plasmoids/org.kde.plasma.konsoleprofiles)

install(FILES package/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-applet-org.kde.plasma.konsoleprofiles.desktop)
