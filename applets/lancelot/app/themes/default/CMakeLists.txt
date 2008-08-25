FILE(GLOB lancelot_svgs *.svg) # libplasma in 4.1 dooen't support svgz extension IGNORE_DIFF
install( FILES ${lancelot_svgs}  DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/lancelot/ )

FILE(GLOB lancelot_configs *.config)
install( FILES ${lancelot_configs}  DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/lancelot/ )

