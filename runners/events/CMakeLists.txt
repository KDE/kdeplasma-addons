project(plasma-events)

find_package(KdepimLibs REQUIRED)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDEPIMLIBS_INCLUDE_DIR} ${KDE4_INCLUDES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

# We add our source code here
set(events_SRCS events.cpp datetime_parser.cpp datetime_range.cpp)

kde4_add_plugin(plasma_runner_events ${events_SRCS})
target_link_libraries(plasma_runner_events ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_AKONADI_LIBS} ${KDEPIMLIBS_KCAL_LIBS})

install(TARGETS plasma_runner_events
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-runner-events.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

# Unit tests
kde4_add_unit_test(datetime_parser_test datetime_parser_test.cpp datetime_parser.cpp datetime_range.cpp)
target_link_libraries(datetime_parser_test ${KDE4_KDEUI_LIBS} QtTest)
