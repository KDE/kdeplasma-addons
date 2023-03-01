set(quicklaunchplugin_SRCS
    quicklaunch_p.cpp
    quicklaunchplugin.cpp)

add_library(quicklaunchplugin SHARED ${quicklaunchplugin_SRCS})
target_link_libraries(quicklaunchplugin
    Qt::Core
    Qt::Qml
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Notifications)

install(TARGETS quicklaunchplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/quicklaunch)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/quicklaunch)
