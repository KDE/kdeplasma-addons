project(plasma-notes)

install(
    FILES notes.svgz
    DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

plasma_install_package(package org.kde.plasma.notes)
