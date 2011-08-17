project(konqprofiles)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.plasma.applet.konqprofiles)

install(FILES plasma-applet-konqprofiles.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

