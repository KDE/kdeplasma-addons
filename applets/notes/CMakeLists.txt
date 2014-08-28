
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.notes)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.notes")

set(notes_SRCS
    plugin/abstractnotemanager.cpp
    plugin/filesystemnotemanager.cpp
    plugin/note.cpp
    plugin/notemanager.cpp
    plugin/documenthandler.cpp
    plugin/notesplugin.cpp
)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/notes)
add_library(notesplugin SHARED ${notes_SRCS})

target_link_libraries(notesplugin
                        Qt5::Core
                        Qt5::Quick)

install(TARGETS notesplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/notes)

install(
    FILES notes.svgz
    DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
