project(konsoleprofilesapplet)

set(konsoleprofilesapplet_SRCS
    konsoleprofilesapplet.cpp)

kde4_add_plugin(plasma_applet_konsoleprofiles ${konsoleprofilesapplet_SRCS})
target_link_libraries(plasma_applet_konsoleprofiles ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_konsoleprofiles DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-konsoleprofiles.desktop DESTINATION ${SERVICES_INSTALL_DIR})
