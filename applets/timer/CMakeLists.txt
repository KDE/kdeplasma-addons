plasma_install_package(package org.kde.plasma.timer)

add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.timer")

set(timer_SRCS
    plugin/timer.cpp
    plugin/timerplugin.cpp
)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/timer)
add_library(timerplugin SHARED ${timer_SRCS})

target_link_libraries(timerplugin
    Qt5::Core
    Qt5::Qml
)

install(TARGETS timerplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/timer)

install(FILES timer.svgz
	DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

