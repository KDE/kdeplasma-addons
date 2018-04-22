set(astronomicalevents_SRCS
    astronomicaleventsplugin.cpp
)

add_library(astronomicalevents MODULE ${astronomicalevents_SRCS})
target_link_libraries(astronomicalevents
    KF5::ConfigCore
    KF5::Holidays
    KF5::CalendarEvents
    KF5::I18n
)

install(TARGETS astronomicalevents  DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasmacalendarplugins)

add_subdirectory(config)
