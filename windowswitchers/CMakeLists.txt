# packages
set(KWIN_NAME "kwin")
install( DIRECTORY big_icons    DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY compact      DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY informative  DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY present_windows  DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY small_icons  DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY text         DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY thumbnail_grid   DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY thumbnails   DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/tabbox )

# service files
install( FILES big_icons/metadata.desktop    DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_big_icons.desktop )
install( FILES compact/metadata.desktop      DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_compact.desktop )
install( FILES informative/metadata.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_informative.desktop )
install( FILES present_windows/metadata.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_present_windows.desktop )
install( FILES small_icons/metadata.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_small_icons.desktop )
install( FILES text/metadata.desktop         DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_text.desktop )
install( FILES thumbnail_grid/metadata.desktop   DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_thumbnail_grid.desktop )
install( FILES thumbnails/metadata.desktop   DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_thumbnails.desktop )

install (FILES IconTabBox.qml DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/tabbox/big_icons/contents/ui)
install (FILES IconTabBox.qml DESTINATION ${KDE_INSTALL_DATADIR}/${KWIN_NAME}/tabbox/small_icons/contents/ui)
