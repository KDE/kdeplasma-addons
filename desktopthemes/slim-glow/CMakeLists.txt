
install(FILES colors metadata.desktop LICENSE DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/)

FILE(GLOB widgets widgets/*.svgz)
install( FILES ${widgets} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/widgets/ )

FILE(GLOB dialogs dialogs/*.svgz)
install( FILES ${dialogs} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/dialogs/ )

FILE(GLOB opaque opaque/widgets/*.svgz)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/opaque/widgets/ )

FILE(GLOB opaque opaque/dialogs/*.svgz)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/opaque/dialogs/ )

# FILE(GLOB lancelot lancelot/*.svgz)
# install( FILES ${lancelot} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/lancelot/ )
  #install(FILES lancelot/theme.config DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/slim-glow/lancelot/ )

