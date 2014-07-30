project(plasmaconverter)

add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_converterrunner\")

set(krunner_converter_SRCS
    converterrunner.cpp
)

add_library(krunner_converter ${krunner_converter_SRCS})
target_link_libraries(krunner_converter KF5::UnitConversion KF5::KIOCore KF5::I18n KF5::Runner)
install(TARGETS krunner_converter DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-converter.desktop DESTINATION ${SERVICES_INSTALL_DIR})

