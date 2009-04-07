set(bubblemon_SRCS
    bubble.cpp
    )

kde4_add_ui_files(bubblemon_SRCS settings.ui)
kde4_add_plugin(plasma_applet_bubblemon ${bubblemon_SRCS})
target_link_libraries(plasma_applet_bubblemon ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_applet_bubblemon DESTINATION ${PLUGIN_INSTALL_DIR})
