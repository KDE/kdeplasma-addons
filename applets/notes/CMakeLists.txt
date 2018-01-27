
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.notes)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.notes")

set(notes_SRCS
    plugin/abstractnoteloader.cpp
    plugin/documenthandler.cpp
    plugin/filesystemnoteloader.cpp
    plugin/note.cpp
    plugin/notemanager.cpp
    plugin/notesplugin.cpp
)

add_library(notesplugin SHARED ${notes_SRCS})

target_link_libraries(notesplugin
                        Qt5::Quick
                        KF5::CoreAddons)

install(TARGETS notesplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/notes)
install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/notes)
