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

kcoreaddons_add_plugin(plasma_applet_comic SOURCES ${comic_SRCS} INSTALL_NAMESPACE "plasma/applets")

target_link_libraries(plasma_applet_comic
                      Qt::Gui
                      Qt::Widgets
                      KF5::Plasma
                      KF5::I18n
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::NewStuff
                      KF5::Notifications
                      KF5::Archive)

install(FILES comic.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
plasma_install_package(package org.kde.plasma.comic)

