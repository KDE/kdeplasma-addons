plasma_install_package(package org.kde.plasma.minimizeall)

add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.minimizeall")

set(minimizeall_SRCS
    plugin/minimizeall.cpp
    plugin/minimizeallplugin.cpp
)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/minimizeall)
add_library(minimizeallplugin SHARED ${minimizeall_SRCS})

target_link_libraries(minimizeallplugin
     Qt5::Core
     Qt5::Qml
     Qt5::Quick
     KF5::WindowSystem
    )

if (Qt5X11Extras_FOUND)
    target_link_libraries(minimizeallplugin
         #Qt5::X11Extras
        )
endif()

install(TARGETS minimizeallplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/minimizeall)
