add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.comic\")

add_subdirectory(engine)

set(comic_SRCS
    comic.cpp
    comicmodel.cpp
    comicupdater.cpp
    checknewstrips.cpp
    comicdata.cpp
    comicinfo.cpp
    comicsaver.cpp
    stripselector.cpp
    activecomicmodel.cpp
)

kcoreaddons_add_plugin(org.kde.plasma.comic SOURCES ${comic_SRCS} INSTALL_NAMESPACE "plasma/applets")

target_link_libraries(org.kde.plasma.comic
                      Qt::Gui
                      Qt::Widgets
                      KF5::Plasma
                      KF5::I18n
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::NewStuffCore
                      KF5::Notifications
                      KF5::XmlGui
                      plasma_engine_comic)

install(FILES comic.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
plasma_install_package(package org.kde.plasma.comic)

