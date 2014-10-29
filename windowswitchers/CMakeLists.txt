# packages
set(KWIN_NAME "kwin")
install( DIRECTORY big_icons    DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY compact      DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY informative  DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY present_windows  DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY small_icons  DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY text         DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/tabbox )
install( DIRECTORY thumbnails   DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/tabbox )

# service files
install( FILES big_icons/metadata.desktop    DESTINATION ${SERVICES_INSTALL_DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_big_icons.desktop )
install( FILES compact/metadata.desktop      DESTINATION ${SERVICES_INSTALL_DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_compact.desktop )
install( FILES informative/metadata.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_informative.desktop )
install( FILES present_windows/metadata.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_present_windows.desktop )
install( FILES small_icons/metadata.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_small_icons.desktop )
install( FILES text/metadata.desktop         DESTINATION ${SERVICES_INSTALL_DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_text.desktop )
install( FILES thumbnails/metadata.desktop   DESTINATION ${SERVICES_INSTALL_DIR}/${KWIN_NAME} RENAME kwin4_window_switcher_thumbnails.desktop )

install (FILES IconTabBox.qml DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/tabbox/big_icons/contents/ui)
install (FILES IconTabBox.qml DESTINATION ${DATA_INSTALL_DIR}/${KWIN_NAME}/tabbox/small_icons/contents/ui)
