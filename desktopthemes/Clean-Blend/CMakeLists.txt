
install(FILES colors metadata.desktop DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Clean-Blend/)

FILE(GLOB widgets widgets/*.svgz)
install( FILES ${widgets} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Clean-Blend/widgets/ )

FILE(GLOB dialogs dialogs/*.svgz)
install( FILES ${dialogs} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Clean-Blend/dialogs/ )

FILE(GLOB opaque opaque/widgets/*.svgz)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Clean-Blend/opaque/widgets/ )

FILE(GLOB opaque opaque/dialogs/*.svgz)
install( FILES ${opaque} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Clean-Blend/opaque/dialogs/ )

FILE(GLOB rssnow rssnow/*.svg)
install( FILES ${rssnow} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Clean-Blend/rssnow/ )

FILE(GLOB stylesheets stylesheets/*.css)
install( FILES ${stylesheets} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Clean-Blend/stylesheets/ )

FILE(GLOB lancelot lancelot/*.svgz)
install( FILES ${lancelot} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Clean-Blend/lancelot/ )

FILE(GLOB weatherstation weatherstation/*.svgz)
install( FILES ${weatherstation} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Clean-Blend/weatherstation/ )

FILE(GLOB systemtray systemtray/*.svgz)
install( FILES ${systemtray} DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/Clean-Blend/systemtray/ )
