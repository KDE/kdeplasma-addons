ecm_install_icons(
    ICONS sc-apps-fifteenpuzzle.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
)

plasma_add_applet(org.kde.plasma.fifteenpuzzle
    QML_SOURCES
        qml/main.qml
        qml/configAppearance.qml
        qml/FifteenPuzzle.qml
        qml/Piece.qml
        qml/config.qml
    CPP_SOURCES
        fifteenimageprovider.cpp
    RESOURCES
        qml/blanksquare.svg
        main.xml
    GENERATE_APPLET_CLASS
)

ecm_qt_declare_logging_category(org.kde.plasma.fifteenpuzzle
    HEADER debug_p.h
    IDENTIFIER "PLASMA_FIFTEENPUZZLE"
    CATEGORY_NAME org.kde.plasma.fifteenpuzzle
    DESCRIPTION "Plasma Fifteen Puzzle"
    DEFAULT_SEVERITY Warning
    EXPORT "PLASMA_FIFTEENPUZZLE"
)

target_link_libraries(org.kde.plasma.fifteenpuzzle PRIVATE Qt::Quick Qt::Qml)
