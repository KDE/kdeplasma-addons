project(groupingdesktop)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_BINARY_DIR}
                    ${KDE4_INCLUDES}
                    ${NEPOMUK_INCLUDES})

add_subdirectory(lib)
add_subdirectory(desktop)
add_subdirectory(panel)