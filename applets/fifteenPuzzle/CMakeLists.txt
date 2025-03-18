ecm_install_icons(
    ICONS sc-apps-fifteenpuzzle.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
)

plasma_install_package(package org.kde.plasma.fifteenpuzzle)

ecm_add_qml_module(fifteenpuzzleplugin URI org.kde.plasma.private.fifteenpuzzle)
target_sources(fifteenpuzzleplugin PRIVATE
    plugin/fifteenimageprovider.cpp
    plugin/fifteenpuzzleplugin.cpp
)
ecm_qt_declare_logging_category(fifteenpuzzleplugin
    HEADER debug_p.h
    IDENTIFIER "PLASMA_FIFTEENPUZZLE"
    CATEGORY_NAME org.kde.plasma.fifteenpuzzle
    DESCRIPTION "Plasma Fifteen Puzzle"
    DEFAULT_SEVERITY Warning
    EXPORT "PLASMA_FIFTEENPUZZLE"
)
target_link_libraries(fifteenpuzzleplugin PRIVATE Qt::Quick Qt::Qml)
ecm_finalize_qml_module(fifteenpuzzleplugin)
