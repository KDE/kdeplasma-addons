add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_datetime")

kcoreaddons_add_plugin(krunner_datetime  SOURCES datetimerunner.cpp INSTALL_NAMESPACE "kf5/krunner")
target_link_libraries(krunner_datetime
    KF5::Runner
    KF5::KIOWidgets
    KF5::I18n
)
