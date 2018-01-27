add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_converterrunner\")

set(krunner_converter_SRCS
    converterrunner.cpp
)

add_library(krunner_converter MODULE ${krunner_converter_SRCS})
target_link_libraries(krunner_converter KF5::UnitConversion KF5::KIOCore KF5::I18n KF5::Runner)
install(TARGETS krunner_converter DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-runner-converter.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

