project(plasma-pastebin)

set(pastebin_SRCS
  backends/server.cpp
  backends/pastebinca.cpp
  backends/pastebincom.cpp
  backends/imagebinca.cpp
  backends/imageshack.cpp
  pastebin.cpp)

kde4_add_ui_files(pastebin_SRCS pastebinConfig.ui)
kde4_add_ui_files(pastebin_SRCS pastebinServersConfig.ui)

kde4_add_plugin(plasma_applet_pastebin ${pastebin_SRCS})
target_link_libraries(plasma_applet_pastebin ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_pastebin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-pastebin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES plasma_pastebin.notifyrc DESTINATION ${DATA_INSTALL_DIR}/plasma_pastebin)

