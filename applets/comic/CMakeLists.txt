project(plasma-comic)

if(HAVE_NEPOMUK)
    add_definitions(-DHAVE_NEPOMUK)
    add_definitions(-DDISABLE_NEPOMUK_LEGACY)
endif(HAVE_NEPOMUK)

set(comic_SRCS
    comic.cpp
    comicmodel.cpp
    configwidget.cpp
    comicarchivejob.cpp
    comicarchivedialog.cpp
    checknewstrips.cpp
    comicdata.cpp
    comicinfo.cpp
    comicsaver.cpp
    stripselector.cpp
    activecomicmodel.cpp
)

kde4_add_ui_files(comic_SRCS
                  comicSettings.ui
                  appearanceSettings.ui
                  advancedsettings.ui
                  comicarchivedialog.ui
)

kde4_add_plugin(plasma_applet_comic ${comic_SRCS})
target_link_libraries(plasma_applet_comic ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KNEWSTUFF3_LIBS} ${QT_QTDECLARATIVE_LIBRARY})

if(HAVE_NEPOMUK)
  target_link_libraries(plasma_applet_comic ${NEPOMUK_LIBRARIES} ${NEPOMUK_UTILS_LIBRARIES} ${SOPRANO_LIBRARIES})
endif(HAVE_NEPOMUK)

install(TARGETS plasma_applet_comic DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-comic-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES comic.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
install(DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.comic)