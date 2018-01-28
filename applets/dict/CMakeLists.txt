find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG COMPONENTS WebEngine)

set_package_properties(Qt5WebEngine PROPERTIES DESCRIPTION "needed by the dict applet"
    URL "https://doc.qt.io/qt-5/qtwebengine-index.html"
)

if (Qt5WebEngine_FOUND)

#######################################################################################
# Package (metadata.desktop and QML)
plasma_install_package(package org.kde.plasma_applet_dict)

#######################################################################################
# Icon for the package
install(FILES accessories-dictionary.svgz
        DESTINATION ${DATA_INSTALL_DIR}/icons/hicolor/scalable/apps/)

#######################################################################################
# C++ Plugin

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

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/dict)
install(TARGETS dictplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/dict)

endif()
