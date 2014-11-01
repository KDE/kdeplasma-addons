project(fifteenpuzzle)

install(FILES fifteenpuzzle.svgz
        DESTINATION ${DATA_INSTALL_DIR}/icons/hicolor/scalable/apps/)

plasma_install_package(package org.kde.plasma.fifteenpuzzle)

set(fifteenpuzzleplugin_SRCS
    plugin/fifteenimageprovider.cpp
    plugin/fifteenpuzzleplugin.cpp
)

add_library(fifteenpuzzleplugin SHARED ${fifteenpuzzleplugin_SRCS})

target_link_libraries(fifteenpuzzleplugin Qt5::Quick Qt5::Qml)
install(TARGETS fifteenpuzzleplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/fifteenpuzzle)
install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/fifteenpuzzle)
