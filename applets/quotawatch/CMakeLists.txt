
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.quotawatch)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.quotawatch")

set(notes_SRCS
    plugin/plugin.cpp
    plugin/quotawatch.cpp
    plugin/QuotaListModel.cpp
    plugin/quotaitem.cpp
)

add_library(quotawatchplugin SHARED ${notes_SRCS})

target_link_libraries(quotawatchplugin
                      Qt5::Quick
                      KF5::CoreAddons
                      KF5::I18n)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/quotawatch)
install(TARGETS quotawatchplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/quotawatch)
