
install(FILES colors metadata.desktop DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Silicon/)

FILE(GLOB widgets widgets/*.svg)
install( FILES ${widgets} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Silicon/widgets/ )

FILE(GLOB dialogs dialogs/*.svg)
install( FILES ${dialogs} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Silicon/dialogs/ )

FILE(GLOB opaque opaque/widgets/*.svg)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Silicon/opaque/widgets/ )

FILE(GLOB opaque opaque/dialogs/*.svg)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Silicon/opaque/dialogs/ )


