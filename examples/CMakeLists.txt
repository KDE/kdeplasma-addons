install(DIRECTORY ugly/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.ugly-qml)
install(FILES ugly/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME ugly-example.desktop)

install(DIRECTORY animals/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.animals)
install(FILES animals/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME animals-example.desktop)
