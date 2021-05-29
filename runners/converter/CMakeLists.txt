add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_converterrunner\")

kcoreaddons_add_plugin(unitconverter SOURCES converterrunner.cpp INSTALL_NAMESPACE "kf5/krunner")
kcoreaddons_desktop_to_json(unitconverter plasma-runner-converter.desktop)
target_link_libraries(unitconverter
        KF5::I18n
        KF5::Runner
        KF5::UnitConversion
        Qt::Widgets
        )

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
