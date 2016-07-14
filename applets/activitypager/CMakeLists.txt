add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.activitypager\")
plasma_install_package(package org.kde.plasma.activitypager)

set(pager_SRCS
    plugin/model.cpp
    plugin/activitypager.cpp
    plugin/activitypagerplugin.cpp)

add_library(activitypagerplugin SHARED ${pager_SRCS})

target_link_libraries(activitypagerplugin
                      KF5::KIOCore
                      KF5::WindowSystem
                      KF5::Plasma
                      KF5::Activities
                      KF5::KDELibs4Support
                      Qt5::Core
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::DBus
)

if (X11_FOUND)
    target_link_libraries(activitypagerplugin Qt5::X11Extras PW::LibTaskManager)
endif()

install(TARGETS activitypagerplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/activitypager)
install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/activitypager)
