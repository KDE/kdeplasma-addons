project(groupingdesktop)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR}
		    ${CMAKE_BINARY_DIR}
		    ${KDE4_INCLUDES}
		    ${NEPOMUK_INCLUDES})

# set(abstractgroup_SRCS
#           abstractgroup.cpp)

set(groupingdesktop_SRCS groupingcontainment.cpp grouphandle.cpp
          groupingdesktop.cpp
          abstractgroup.cpp
        gridlayout.cpp
            )

# set(gridlayout_SRCS
#         gridlayout.cpp)

# kde4_add_library(plasma_abstractgroup SHARED ${abstractgroup_SRCS})
kde4_add_plugin(plasma_containment_groupingdesktop ${groupingdesktop_SRCS})
# kde4_add_plugin(plasma_group_gridlayout ${gridlayout_SRCS})
# target_link_libraries(plasma_abstractgroup ${KDE4_PLASMA_LIBS})
target_link_libraries(plasma_containment_groupingdesktop ${KDE4_PLASMA_LIBS} )
# target_link_libraries(plasma_group_gridlayout ${KDE4_PLASMA_LIBS} plasma_abstractgroup)

# set_target_properties(plasma_abstractgroup PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )

# install(TARGETS plasma_abstractgroup EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS plasma_containment_groupingdesktop DESTINATION ${PLUGIN_INSTALL_DIR})
# install(TARGETS plasma_group_gridlayout DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-groupingdesktop.desktop DESTINATION ${SERVICES_INSTALL_DIR})
# install(FILES plasma-group-gridlayout.desktop DESTINATION ${SERVICES_INSTALL_DIR})
# install(FILES plasma-group.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})