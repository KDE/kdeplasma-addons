add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_katesessions\")

set(krunner_katesessions_SRCS katesessions.cpp)
add_library(krunner_katesessions MODULE ${krunner_katesessions_SRCS})
kcoreaddons_desktop_to_json(krunner_katesessions plasma-runner-katesessions.desktop)
target_link_libraries(krunner_katesessions KF5::KIOGui KF5::Notifications KF5::I18n KF5::Runner)

install(TARGETS krunner_katesessions DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/krunner)
