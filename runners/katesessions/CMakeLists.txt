add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_katesessions\")

kcoreaddons_add_plugin(krunner_katesessions SOURCES katesessions.cpp INSTALL_NAMESPACE "kf5/krunner")
kcoreaddons_desktop_to_json(krunner_katesessions plasma-runner-katesessions.desktop)
target_link_libraries(krunner_katesessions KF5::KIOGui KF5::Notifications KF5::I18n KF5::Runner)
