add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_konsoleprofiles")

kcoreaddons_add_plugin(krunner_konsoleprofiles SOURCES konsoleprofiles.cpp INSTALL_NAMESPACE "kf5/krunner")
kcoreaddons_desktop_to_json(krunner_konsoleprofiles plasma-runner-konsoleprofiles.desktop)
target_link_libraries(krunner_konsoleprofiles
    KF5::Runner
    KF5::KIOGui
    KF5::I18n
    KF5::Notifications
)
