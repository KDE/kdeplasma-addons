plasma_add_applet(org.kde.plasma.colorpicker
    QML_SOURCES
        qml/ColorCircle.qml
        qml/ColorContextMenu.qml
        qml/CompactRepresentation.qml
        qml/configGeneral.qml
        qml/config.qml
        qml/ImageColors.qml
        qml/LoadingIndicator.qml
        qml/main.qml
    CPP_SOURCES
        grabwidget.cpp
        utils.cpp
    RESOURCES
        main.xml
        qml/Logic.js
    GENERATE_APPLET_CLASS
)

target_link_libraries(org.kde.plasma.colorpicker PRIVATE
    Qt::DBus
    Qt::Gui
    Qt::Qml
    Qt::Widgets
)
