project(konqprofilesapplet)


set(konqprofilesapplet_SRCS
    konqprofilesapplet.cpp)

kde4_add_plugin(plasma_applet_konqprofiles ${konqprofilesapplet_SRCS})
target_link_libraries(plasma_applet_konqprofiles ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_konqprofiles DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-konqprofiles.desktop DESTINATION ${SERVICES_INSTALL_DIR})
