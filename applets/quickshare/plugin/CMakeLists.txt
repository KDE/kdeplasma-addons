install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/purpose)

set(purposeplugin_SRCS
    purposeplugin.cpp
    contenttracker.cpp
)

set(activitymanagerslc_xml org.kde.ActivityManager.SLC.xml)
qt5_add_dbus_interface(purposeplugin_SRCS ${activitymanagerslc_xml} activitymanagerslc_interface)

add_library(purposeplugin SHARED ${purposeplugin_SRCS})

target_link_libraries(purposeplugin
                        Qt5::Quick
                        Qt5::DBus
                        KF5::CoreAddons)

install(TARGETS purposeplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/purpose)
