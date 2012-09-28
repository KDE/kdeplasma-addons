install(DIRECTORY animals/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.animals)
install(FILES animals/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME animals-example.desktop)
