find_package(Qt5WebEngine ${QT_MIN_VERSION} CONFIG)

set_package_properties(Qt5WebEngine PROPERTIES
    PURPOSE "Needed by the dict applet"
    URL "https://doc.qt.io/qt-5/qtwebengine-index.html"
    TYPE OPTIONAL
)

if (Qt5WebEngine_FOUND)

#######################################################################################
# Package (metadata.desktop and QML)
plasma_install_package(package org.kde.plasma_applet_dict)
# TODO: fix pluginid "org.kde.plasma_applet_dict" into properly namespaced id "org.kde.plasma.dict"

#######################################################################################
# Icon for the package
ecm_install_icons(
    ICONS sc-apps-accessories-dictionary.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
)

#######################################################################################
# C++ Plugin

add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma_applet_dict")

set(dict_SRCS
  plugin/dict_plugin.cpp
  plugin/dict_object.cpp
  plugin/dictionariesmodel.cpp
)

add_library(dictplugin SHARED ${dict_SRCS})
target_link_libraries(dictplugin
    KF5::Plasma
    KF5::I18n
    Qt5::Quick
    Qt5::WebEngine
)

install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/dict)
install(TARGETS dictplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/dict)

endif()
