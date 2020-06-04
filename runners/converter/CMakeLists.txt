add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_converterrunner\")

set(krunner_converter_SRCS converterrunner.cpp)

add_library(krunner_converter_static STATIC ${krunner_converter_SRCS})
target_link_libraries(krunner_converter_static
        KF5::I18n
        KF5::Runner
        KF5::UnitConversion
        Qt5::Widgets
        )

add_library(krunner_converter MODULE plugin.cpp)
kcoreaddons_desktop_to_json(krunner_converter plasma-runner-converter.desktop )
target_link_libraries(krunner_converter krunner_converter_static)

install(TARGETS krunner_converter DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/krunner)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
