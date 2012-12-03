project(plasma-events)

find_package(KdepimLibs REQUIRED)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${Boost_INCLUDE_DIR} ${KDEPIMLIBS_INCLUDE_DIRS} ${KDE4_INCLUDES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

# Plasma runner
set(events_SRCS events.cpp datetime_parser.cpp datetime_range.cpp collection_selector.cpp)

kde4_add_plugin(plasma_runner_events ${events_SRCS})
target_link_libraries(plasma_runner_events ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_AKONADI_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS})
add_dependencies(plasma_runner_events kcm_plasma_runner_events)

# Config module
set(kcm_events_SRCS events_config.cpp collection_selector.cpp)

kde4_add_ui_files(kcm_events_SRCS events_config.ui)
kde4_add_plugin(kcm_plasma_runner_events ${kcm_events_SRCS})
target_link_libraries(kcm_plasma_runner_events ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_AKONADI_LIBS} ${KDEPIMLIBS_KCALCORE_LIBS})

# Installation
install(TARGETS plasma_runner_events kcm_plasma_runner_events
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-runner-events.desktop plasma-runner-events_config.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

# Unit tests
kde4_add_unit_test(datetime_parser_test TESTNAME plasma-runner-events-datetime_parser_test datetime_parser_test.cpp datetime_parser.cpp datetime_range.cpp)
target_link_libraries(datetime_parser_test ${KDE4_KDEUI_LIBS} ${QT_QTTEST_LIBRARY} )

