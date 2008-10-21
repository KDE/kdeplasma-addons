project(plasma-fileWatcher)
include_directories( ${KDE4_INCLUDES} )

set(fileWatcher_SRCS fileWatcher.cpp )

kde4_add_ui_files(fileWatcher_SRCS fileWatcherConfig.ui )
kde4_add_plugin(plasma_applet_fileWatcher ${fileWatcher_SRCS})

target_link_libraries(plasma_applet_fileWatcher ${PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_fileWatcher DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-fileWatcher-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
