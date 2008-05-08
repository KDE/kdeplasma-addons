
install(FILES metadata.desktop DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Aya/)

FILE(GLOB widgets widgets/*.svg)
install( FILES ${widgets} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Aya/widgets/ )

FILE(GLOB dialogs dialogs/*.svg)
install( FILES ${dialogs} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Aya/dialogs/ )

FILE(GLOB opaque opaque/widgets/*.svg)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Aya/opaque/widgets/ )

FILE(GLOB opaque opaque/dialogs/*.svg)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Aya/opaque/dialogs/ )


