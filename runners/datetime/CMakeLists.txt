add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_datetime")

set(krunner_datetime_SRCS
    datetimerunner.cpp
)

add_library(krunner_datetime MODULE ${krunner_datetime_SRCS})
kcoreaddons_desktop_to_json(krunner_datetime plasma-runner-datetime.desktop )
target_link_libraries(krunner_datetime
    KF5::Runner
    KF5::KIOWidgets
    KF5::I18n
)

install(TARGETS krunner_datetime DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/krunner)
