project(plasma-comic)

set(comic_SRCS
    comic.cpp
    configwidget.cpp)

kde4_add_plugin(plasma_applet_comic ${comic_SRCS})
target_link_libraries(plasma_applet_comic ${KDE4_KDEUI_LIBS} ${PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_comic DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-comic-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
