project(translator)

install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/translator)

install(FILES plasma-applet-translator-qml.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
