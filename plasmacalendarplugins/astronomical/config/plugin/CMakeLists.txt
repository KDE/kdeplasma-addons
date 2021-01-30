add_library(plasmacalendarastronomicaleventsconfig SHARED configplugin.cpp configstorage.cpp)

target_link_libraries(plasmacalendarastronomicaleventsconfig
    Qt::Qml
    Qt::Core
    KF5::ConfigCore
)

install(TARGETS plasmacalendarastronomicaleventsconfig DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasmacalendar/astronomicaleventsconfig)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasmacalendar/astronomicaleventsconfig)
