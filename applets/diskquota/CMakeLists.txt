
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.diskquota)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.diskquota")

ecm_add_qml_module(diskquotaplugin
    URI org.kde.plasma.private.diskquota
    DEPENDENCIES QtCore
    GENERATE_PLUGIN_SOURCE
)

target_sources(diskquotaplugin PRIVATE
    DiskQuota.cpp
    QuotaListModel.cpp
    QuotaItem.cpp
)

target_link_libraries(diskquotaplugin PRIVATE
    Qt::Quick
    KF6::CoreAddons
    KF6::I18n
)

ecm_finalize_qml_module(diskquotaplugin)
