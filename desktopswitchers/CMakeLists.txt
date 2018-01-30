# packages
set(KWIN_NAME "kwin")
install( DIRECTORY previews DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/desktoptabbox )

# service files
install( FILES previews/metadata.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/${KWIN_NAME} RENAME kwin4_desktop_switcher_previews.desktop )
