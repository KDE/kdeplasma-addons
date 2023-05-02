ecm_add_qml_module(nightcolorcontrolplugin URI org.kde.plasma.private.nightcolorcontrol)
target_sources(nightcolorcontrolplugin PRIVATE
    inhibitor.cpp
    monitor.cpp
    plugin.cpp
)
target_link_libraries(nightcolorcontrolplugin
    Qt::Core
    Qt::DBus
    Qt::Gui
    Qt::Qml
)
ecm_finalize_qml_module(nightcolorcontrolplugin DESTINATION ${KDE_INSTALL_QMLDIR})
