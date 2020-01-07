set(nightcolorcontrolplugin_SOURCES
    inhibitor.cpp
    monitor.cpp
    plugin.cpp
)

add_library(nightcolorcontrolplugin SHARED ${nightcolorcontrolplugin_SOURCES})
target_link_libraries(nightcolorcontrolplugin
    Qt5::Core
    Qt5::DBus
    Qt5::Gui
    Qt5::Qml
)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/nightcolorcontrol)
install(TARGETS nightcolorcontrolplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/nightcolorcontrol)
