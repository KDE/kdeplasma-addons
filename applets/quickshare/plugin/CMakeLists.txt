set(activitymanagerslc_xml org.kde.ActivityManager.SLC.xml)
qt_add_dbus_interface(purposeplugin_SRCS ${activitymanagerslc_xml} activitymanagerslc_interface)

ecm_add_qml_module(purposeplugin URI org.kde.plasma.private.purpose)
target_sources(purposeplugin PRIVATE purposeplugin.cpp contenttracker.cpp mimedatabase.cpp ${purposeplugin_SRCS})
target_link_libraries(purposeplugin PRIVATE
    Qt::Quick
    Qt::DBus
    KF6::CoreAddons
)
ecm_finalize_qml_module(purposeplugin)
