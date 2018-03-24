find_package(Qt5WebEngine ${QT_MIN_VERSION} CONFIG)

set_package_properties(Qt5WebEngine PROPERTIES
    PURPOSE "Needed by the web browser applet"
    URL "https://doc.qt.io/qt-5/qtwebengine-index.html"
    TYPE OPTIONAL
)

if (Qt5WebEngine_FOUND)

plasma_install_package(package org.kde.plasma.webbrowser)

endif()
