add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_converterrunner\")

add_library(unitconverter MODULE converterrunner.cpp)
kcoreaddons_desktop_to_json(unitconverter plasma-runner-converter.desktop)
target_link_libraries(unitconverter
        KF5::I18n
        KF5::Runner
        KF5::UnitConversion
        Qt::Widgets
        )

install(TARGETS unitconverter DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/krunner)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
