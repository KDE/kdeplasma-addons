plasma_install_package(package org.kde.plasma.colorpicker)

set(colorpickerplugin_SRCS
    plugin/grabwidget.cpp
    plugin/colorpickerplugin.cpp
)

install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/colorpicker)
add_library(colorpickerplugin SHARED ${colorpickerplugin_SRCS})

target_link_libraries(colorpickerplugin Qt::DBus Qt::Gui Qt::Qml Qt::Widgets KF5::WindowSystem)
install(TARGETS colorpickerplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/colorpicker)
