
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.notes)


#######################################################################################
# Notes Library

set(notes_SRCS
    plugin/documenthandler.cpp
    plugin/notesplugin.cpp
)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/notes)
add_library(notesplugin SHARED ${notes_SRCS})

target_link_libraries(notesplugin
                        Qt5::Quick)

install(TARGETS notesplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/notes)

install(
    FILES notes.svgz
    DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

