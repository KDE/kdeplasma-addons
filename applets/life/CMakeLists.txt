project(life)

set(life_SRCS life.cpp)

kde4_add_ui_files(life_SRCS lifeConfig.ui)
kde4_add_plugin(plasma_applet_life ${life_SRCS})
target_link_libraries(plasma_applet_life
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_life
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-life.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

kde4_install_icons(${ICON_INSTALL_DIR})
