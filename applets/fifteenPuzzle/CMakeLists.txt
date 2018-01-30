ecm_install_icons(
    ICONS sc-apps-fifteenpuzzle.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
)

plasma_install_package(package org.kde.plasma.fifteenpuzzle)

set(fifteenpuzzleplugin_SRCS
    plugin/fifteenimageprovider.cpp
    plugin/fifteenpuzzleplugin.cpp
)

add_library(fifteenpuzzleplugin SHARED ${fifteenpuzzleplugin_SRCS})

target_link_libraries(fifteenpuzzleplugin Qt5::Quick Qt5::Qml)
install(TARGETS fifteenpuzzleplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/fifteenpuzzle)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/fifteenpuzzle)
