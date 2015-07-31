
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.diskquota)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.diskquota")

set(diskquota_SRCS
    plugin/plugin.cpp
    plugin/DiskQuota.cpp
    plugin/QuotaListModel.cpp
    plugin/QuotaItem.cpp
)

add_library(diskquotaplugin SHARED ${diskquota_SRCS})

target_link_libraries(diskquotaplugin
                      Qt5::Quick
                      KF5::CoreAddons
                      KF5::I18n)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/diskquota)
install(TARGETS diskquotaplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/diskquota)

install(FILES "icons/quota.svg"
        DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/icons/)