# packages
set(KWIN_NAME "kwin")
install( DIRECTORY previews DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/desktoptabbox )

# service files
install( FILES previews/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR}/${KWIN_NAME} RENAME kwin4_desktop_switcher_previews.desktop )
