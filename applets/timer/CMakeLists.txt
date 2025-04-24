plasma_add_applet(org.kde.plasma.timer
    QML_SOURCES
        CompactRepresentation.qml
        configGeneral.qml
        config.qml
        configTimes.qml
        main.qml
        TimerEdit.qml
        TimerView.qml
    RESOURCES
        main.xml
    CPP_SOURCES
        timer.h
    GENERATE_APPLET_CLASS
)

target_link_libraries(org.kde.plasma.timer PRIVATE Qt::Core Qt::Qml)

install(FILES timer.svgz DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
install(FILES plasma_applet_timer.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
