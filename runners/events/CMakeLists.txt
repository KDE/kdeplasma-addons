project(plasma-events)

# Find the required Libaries
find_package(KDE4 REQUIRED)
find_package(KdepimLibs REQUIRED)

include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDEPIMLIBS_INCLUDE_DIR} ${KDE4_INCLUDES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

# We add our source code here
set(events_SRCS events.cpp datetime_parser.cpp)

kde4_add_plugin(krunner_events ${events_SRCS})
target_link_libraries(krunner_events ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_AKONADI_LIBS} ${KDEPIMLIBS_KCAL_LIBS})

install(TARGETS krunner_events
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-runner-events.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
