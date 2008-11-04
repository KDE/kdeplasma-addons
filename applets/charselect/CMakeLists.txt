project(charselect)

set(charselect_SRCS charselect.cpp)

kde4_add_plugin(plasma_applet_charselect ${charselect_SRCS})
target_link_libraries(plasma_applet_charselect ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_charselect
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-charselect.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
