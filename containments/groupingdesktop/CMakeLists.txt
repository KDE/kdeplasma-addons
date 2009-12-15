project(groupingdesktop)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR}
                    ${CMAKE_BINARY_DIR}
                    ${KDE4_INCLUDES}
                    ${NEPOMUK_INCLUDES}
                    ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal)

set(groupingcontainment_SRCS
        lib/groupingcontainment.cpp
        lib/grouphandle.cpp
        lib/abstractgroup.cpp
        lib/floatinggroup.cpp
        lib/gridgroup.cpp
        lib/itemoverlay.cpp)

set(groupingdesktop_SRCS desktop/groupingdesktop.cpp)

set(groupingpanel_SRCS panel/groupingpanel.cpp)

kde4_add_library(plasma_groupingcontainment SHARED ${groupingcontainment_SRCS})
kde4_add_plugin(plasma_containment_groupingdesktop ${groupingdesktop_SRCS})
kde4_add_plugin(plasma_containment_groupingpanel ${groupingpanel_SRCS})

target_link_libraries(plasma_groupingcontainment ${KDE4_PLASMA_LIBS})
target_link_libraries(plasma_containment_groupingdesktop ${KDE4_PLASMA_LIBS} plasma_groupingcontainment)
target_link_libraries(plasma_containment_groupingpanel ${KDE4_PLASMA_LIBS} plasma_groupingcontainment kephal)

set_target_properties(plasma_groupingcontainment PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS plasma_groupingcontainment EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS plasma_containment_groupingdesktop DESTINATION ${PLUGIN_INSTALL_DIR})
install(TARGETS plasma_containment_groupingpanel DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES desktop/plasma-containment-groupingdesktop.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES panel/plasma-containment-groupingpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR})