kcoreaddons_add_plugin(astronomicalevents SOURCES astronomicaleventsplugin.cpp INSTALL_NAMESPACE "plasmacalendarplugins")
target_link_libraries(astronomicalevents
    KF5::ConfigCore
    KF5::Holidays
    KF5::CalendarEvents
    KF5::I18n
)

add_subdirectory(config)
