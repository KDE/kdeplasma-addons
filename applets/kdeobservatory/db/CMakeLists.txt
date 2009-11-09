FILE(GLOB DATABASE "kdeobservatory.db")
install(FILES ${DATABASE} DESTINATION ${DATA_INSTALL_DIR}/kdeobservatory/data/)
