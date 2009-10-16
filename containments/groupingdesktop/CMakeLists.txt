project(groupingdesktop)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR}
		    ${CMAKE_BINARY_DIR}
		    ${KDE4_INCLUDES}
		    ${NEPOMUK_INCLUDES})

set(groupingdesktop_SRCS
          abstractgroup.cpp
          gridlayout.cpp
          groupingdesktop.cpp
            )

kde4_add_plugin(plasma_containment_groupingdesktop ${groupingdesktop_SRCS})
target_link_libraries(plasma_containment_groupingdesktop ${KDE4_PLASMA_LIBS} )

install(TARGETS plasma_containment_groupingdesktop DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-groupingdesktop.desktop DESTINATION ${SERVICES_INSTALL_DIR})
