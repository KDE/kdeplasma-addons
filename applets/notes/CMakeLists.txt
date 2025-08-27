add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.notes")

plasma_install_package(package org.kde.plasma.notes)

ecm_add_qml_module(notesplugin
    URI org.kde.plasma.private.notes
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick
)

target_sources(notesplugin PRIVATE
    abstractnoteloader.cpp
    documenthandler.cpp
    filesystemnoteloader.cpp
    note.cpp
    notemanager.cpp
    noteshelper.cpp
)
target_link_libraries(notesplugin PRIVATE
    Qt::Quick
    KF6::CoreAddons
    KF6::KIOCore
)
ecm_finalize_qml_module(notesplugin)
