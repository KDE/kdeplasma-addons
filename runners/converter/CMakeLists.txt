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
target_link_libraries(krunner_converter krunner_converter_static)

install(TARGETS krunner_converter DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES plasma-runner-converter.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
