plasma_install_package(package org.kde.plasma.mediaframe)

ecm_add_qml_module(mediaframeplugin URI org.kde.plasma.private.mediaframe)
target_sources(mediaframeplugin PRIVATE
    plugin/mediaframe.cpp
    plugin/mediaframeplugin.cpp
)
ecm_qt_declare_logging_category(mediaframeplugin
    HEADER debug_p.h
    IDENTIFIER "PLASMA_MEDIAFRAME"
    CATEGORY_NAME org.kde.plasma.mediaframe
    DESCRIPTION "Plasma Media Frame widget"
    DEFAULT_SEVERITY Warning
    EXPORT "PLASMA_MEDIAFRAME"
)

target_link_libraries(mediaframeplugin PRIVATE
    Qt::Core
    Qt::Qml
    Qt::Quick
    KF6::I18n
    KF6::KIOCore
)
ecm_finalize_qml_module(mediaframeplugin)
