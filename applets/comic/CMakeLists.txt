project(plasma-comic)

set(comic_SRCS
    comic.cpp
    configwidget.cpp
    fullviewwidget.cpp
    fadingitem.cpp
    imagewidget.cpp
    arrowwidget.cpp)

kde4_add_ui_files( comic_SRCS comicSettings.ui appearanceSettings.ui )

kde4_add_plugin(plasma_applet_comic ${comic_SRCS})
target_link_libraries(plasma_applet_comic ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KNEWSTUFF2_LIBS} )

configure_file(config-nepomuk.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-nepomuk.h)
if(HAVE_NEPOMUK)
  target_link_libraries(plasma_applet_comic ${NEPOMUK_LIBRARIES} ${SOPRANO_LIBRARIES})
endif(HAVE_NEPOMUK)

install(TARGETS plasma_applet_comic DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-comic-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES comic.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
