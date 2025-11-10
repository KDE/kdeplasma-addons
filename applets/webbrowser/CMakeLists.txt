find_package(Qt6WebEngineQuick ${QT_MIN_VERSION} CONFIG)

set_package_properties(Qt6WebEngineQuick PROPERTIES
    PURPOSE "Needed by the web browser applet"
    URL "https://doc.qt.io/qt-6/qtwebengine-index.html"
    TYPE OPTIONAL
)

if (Qt6WebEngineQuick_FOUND)
    plasma_add_applet(org.kde.plasma.webbrowser GENERATE_APPLET_CLASS
        QML_SOURCES
            qml/main.qml
            qml/ConfigAppearance.qml
            qml/ConfigGeneral.qml
            qml/config.qml
        RESOURCES
            main.xml
    )
endif()
