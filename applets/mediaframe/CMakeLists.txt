plasma_add_applet(org.kde.plasma.mediaframe GENERATE_APPLET_CLASS
    QML_SOURCES
        qml/main.qml
        qml/ConfigBehavior.qml
        qml/ConfigMedia.qml
        qml/config.qml
    CPP_SOURCES
        mediaframe.cpp
    RESOURCES
        main.xml
)

ecm_qt_declare_logging_category(org.kde.plasma.mediaframe
    HEADER debug_p.h
    IDENTIFIER "PLASMA_MEDIAFRAME"
    CATEGORY_NAME org.kde.plasma.mediaframe
    DESCRIPTION "Plasma Media Frame widget"
    DEFAULT_SEVERITY Warning
    EXPORT "PLASMA_MEDIAFRAME"
)

target_link_libraries(org.kde.plasma.mediaframe PRIVATE
    Qt::Core
    Qt::Qml
    Qt::Quick
    KF6::I18n
    KF6::KIOCore
)
