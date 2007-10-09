project(plasma-ebn)

set(ebnapplet_SRCS
    ebnapplet.cpp)


kde4_add_plugin(plasma_applet_ebn ${ebnapplet_SRCS})
target_link_libraries(plasma_applet_ebn ${PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_ebn DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-ebn.desktop DESTINATION ${SERVICES_INSTALL_DIR})
