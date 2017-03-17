add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_katesessions\")

set(krunner_katesessions_SRCS
    katesessions.cpp
)


add_library(krunner_katesessions MODULE ${krunner_katesessions_SRCS})
target_link_libraries(krunner_katesessions KF5::KIOCore KF5::I18n KF5::Runner)
install(TARGETS krunner_katesessions DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-katesessions.desktop DESTINATION ${SERVICES_INSTALL_DIR})
