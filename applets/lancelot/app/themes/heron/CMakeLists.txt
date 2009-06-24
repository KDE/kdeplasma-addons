FILE(GLOB lancelot_svgs *.svgz)
install( FILES ${lancelot_svgs}  DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/heron/lancelot/ )

FILE(GLOB lancelot_configs *.config)
install( FILES ${lancelot_configs}  DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/heron/lancelot/ )

