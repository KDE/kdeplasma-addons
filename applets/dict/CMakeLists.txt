#######################################################################################
# Package (metadata.desktop and QML)
plasma_install_package(package org.kde.plasma_applet_dict)

#######################################################################################
# Icon for the package
install(FILES accessories-dictionary.svgz
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)

#######################################################################################
# C++ Plugin

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED WebEngine)

add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.dict")

set(dict_SRCS
  plugin/dict_plugin.cpp
  plugin/dict_object.cpp
  plugin/dictionariesmodel.cpp
)

add_library(dictplugin SHARED ${dict_SRCS})
target_link_libraries(dictplugin
    KF5::Plasma
    KF5::IconThemes
    KF5::I18n
    Qt5::Quick
    Qt5::WebEngine
)

install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/dict)
install(TARGETS dictplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/dict)

