FILE(GLOB lancelot_svgs *.svgz)
install( FILES ${lancelot_svgs}  DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/oxygen/lancelot/ )

FILE(GLOB lancelot_configs *.config)
install( FILES ${lancelot_configs}  DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/oxygen/lancelot/ )

