project(binaryclock)

set(binaryclock_SRCS binaryclock.cpp)

kde4_add_ui_files(binaryclock_SRCS clockConfig.ui)
kde4_add_plugin(plasma_applet_binaryclock ${binaryclock_SRCS})

target_link_libraries(plasma_applet_binaryclock ${KDE4WORKSPACE_PLASMACLOCK_LIBRARY} ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_binaryclock DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-binaryclock.desktop DESTINATION ${SERVICES_INSTALL_DIR})
