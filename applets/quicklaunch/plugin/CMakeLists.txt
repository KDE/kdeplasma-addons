set(quicklaunchplugin_SRCS
    quicklaunch_p.cpp
    quicklaunchplugin.cpp)

add_library(quicklaunchplugin SHARED ${quicklaunchplugin_SRCS})
target_link_libraries(quicklaunchplugin
    Qt5::Core
    Qt5::Qml
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Notifications)

install(TARGETS quicklaunchplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/quicklaunch)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/quicklaunch)
