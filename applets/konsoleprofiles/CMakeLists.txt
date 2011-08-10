project(konsoleprofiles)

install(DIRECTORY package/
        DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.plasma.applet.konsoleprofiles)

install(FILES plasma-applet-konsoleprofiles.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

