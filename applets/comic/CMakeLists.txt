project(plasma-comic)


set(comic_SRCS
    comic.cpp
    comicmodel.cpp
    comicupdater.cpp
    comicarchivejob.cpp
    comicarchivedialog.cpp
    checknewstrips.cpp
    comicdata.cpp
    comicinfo.cpp
    comicsaver.cpp
    stripselector.cpp
    activecomicmodel.cpp
)

ki18n_wrap_ui(comic_SRCS
                  comicSettings.ui
                  appearanceSettings.ui
                  advancedsettings.ui
                  comicarchivedialog.ui
)


add_library(plasma_applet_comic MODULE ${comic_SRCS})

kcoreaddons_desktop_to_json(plasma_applet_comic package/metadata.desktop)
#file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/metadata.json ${CMAKE_CURRENT_BINARY_DIR}/plasma-comic-default.json)

target_link_libraries(plasma_applet_comic
                      KF5::Plasma
                      KF5::Solid
                      KF5::KIOWidgets
                      KF5::NewStuff
                      KF5::Archive
                      KF5::KDELibs4Support)


install(TARGETS plasma_applet_comic DESTINATION ${PLUGIN_INSTALL_DIR})
#install(FILES plasma-comic-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES comic.knsrc DESTINATION ${CONFIG_INSTALL_DIR})
plasma_install_package(package org.kde.plasma.comic)

