add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.comic\")

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

ki18n_wrap_ui(comic_SRCS comicarchivedialog.ui
)


add_library(plasma_applet_comic MODULE ${comic_SRCS})

kcoreaddons_desktop_to_json(plasma_applet_comic package/metadata.desktop SERVICE_TYPES plasma-applet.desktop)

target_link_libraries(plasma_applet_comic
                      Qt5::Gui
                      Qt5::Widgets
                      KF5::Plasma
                      KF5::I18n
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::NewStuff
                      KF5::Notifications
                      KF5::Archive)


install(TARGETS plasma_applet_comic DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/applets)
#install(FILES plasma-comic-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES comic.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
plasma_install_package(package org.kde.plasma.comic)

