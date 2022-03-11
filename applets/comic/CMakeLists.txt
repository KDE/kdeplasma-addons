add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.comic\")

add_subdirectory(engine)

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

kcoreaddons_add_plugin(org.kde.plasma.comic SOURCES ${comic_SRCS} INSTALL_NAMESPACE "plasma/applets")

target_link_libraries(org.kde.plasma.comic
                      Qt::Gui
                      Qt::Widgets
                      KF5::Plasma
                      KF5::I18n
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::NewStuff
                      KF5::Notifications
                      KF5::Archive
                      plasma_engine_comic)

install(FILES comic.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
plasma_install_package(package org.kde.plasma.comic)

