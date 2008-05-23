
FILE(GLOB lancelot *.svg)
install( FILES ${lancelot}  DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/lancelot/ )
install( FILES theme.config DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/lancelot/ )

