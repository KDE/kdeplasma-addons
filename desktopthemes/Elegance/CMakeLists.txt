
install(FILES colors metadata.desktop DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Elegance/)

FILE(GLOB widgets widgets/*.svg)
install( FILES ${widgets} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Elegance/widgets/ )

FILE(GLOB dialogs dialogs/*.svg)
install( FILES ${dialogs} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Elegance/dialogs/ )

FILE(GLOB opaque opaque/widgets/*.svg)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Elegance/opaque/widgets/ )

FILE(GLOB opaque opaque/dialogs/*.svg)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Elegance/opaque/dialogs/ )


