plasma_add_applet(org.kde.plasma.quicklaunch
    QML_SOURCES
        ConfigGeneral.qml
        config.qml
        IconItem.qml
        main.qml
        Popup.qml
        UrlModel.qml
        layout.js
    CPP_SOURCES
        quicklaunch_p.cpp
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)

target_link_libraries(org.kde.plasma.quicklaunch PRIVATE KF6::KIOWidgets KF6::JobWidgets)
