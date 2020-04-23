########### next target ###############

set(dbus_interface_name org.kde.potd.xml)
set(kded_potd_PART_SRCS kded_potd.cpp)

add_library(kded_potd MODULE ${kded_potd_PART_SRCS})
set_target_properties(kded_potd PROPERTIES OUTPUT_NAME potd)

target_link_libraries(kded_potd
   KF5::DBusAddons
   KF5::Plasma
)

########### install files ###############
kcoreaddons_desktop_to_json(kded_potd kded_potd.desktop)
install(TARGETS kded_potd DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/kded)
