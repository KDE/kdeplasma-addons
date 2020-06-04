add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_konsoleprofiles")

set(krunner_konsoleprofiles_SRCS
    konsoleprofiles.cpp
)

add_library(krunner_konsoleprofiles MODULE ${krunner_konsoleprofiles_SRCS})
kcoreaddons_desktop_to_json(krunner_konsoleprofiles plasma-runner-konsoleprofiles.desktop)
target_link_libraries(krunner_konsoleprofiles
    KF5::Runner
    KF5::KIOGui
    KF5::I18n
    KF5::Notifications
)

install(TARGETS krunner_konsoleprofiles DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/krunner)
