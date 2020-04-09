add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_konsoleprofiles")

set(krunner_konsoleprofiles_SRCS
    konsoleprofiles.cpp
)

add_library(krunner_konsoleprofiles MODULE ${krunner_konsoleprofiles_SRCS})
target_link_libraries(krunner_konsoleprofiles
    KF5::Runner
    KF5::KIOGui
    KF5::I18n
    KF5::Notifications
)

install(TARGETS krunner_konsoleprofiles DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES plasma-runner-konsoleprofiles.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
