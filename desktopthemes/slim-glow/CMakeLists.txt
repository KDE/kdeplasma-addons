
install(FILES colors metadata.desktop LICENSE DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/)

FILE(GLOB widgets widgets/*.svg)
install( FILES ${widgets} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/widgets/ )

FILE(GLOB dialogs dialogs/*.svg)
install( FILES ${dialogs} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/dialogs/ )

FILE(GLOB opaque opaque/widgets/*.svg)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/opaque/widgets/ )

FILE(GLOB opaque opaque/dialogs/*.svg)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/opaque/dialogs/ )

FILE(GLOB lancelot lancelot/*.svg)
install( FILES ${lancelot} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/lancelot/ )
install(FILES lancelot/theme.config DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/lancelot/ )

