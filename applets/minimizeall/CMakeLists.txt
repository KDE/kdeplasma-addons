
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.minimizeall")

set(minimizeall_SRCS
    plugin/minimizeall.cpp
    plugin/minimizeallplugin.cpp
)

add_library(minimizeallplugin SHARED ${minimizeall_SRCS})

target_link_libraries(minimizeallplugin
     Qt5::Core
     Qt5::Qml
     Qt5::Quick
     KF5::WindowSystem
    )

install(TARGETS minimizeallplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/minimizeall)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/minimizeall)

plasma_install_package(package org.kde.plasma.minimizeall)
