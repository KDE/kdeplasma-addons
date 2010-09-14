
set(griddesktop_SRCS griddesktop.cpp)

kde4_add_plugin(plasma_containment_griddesktop ${griddesktop_SRCS})
target_link_libraries(plasma_containment_griddesktop ${KDE4_PLASMA_LIBS} plasma_groupingcontainment)

install(TARGETS plasma_containment_griddesktop DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-griddesktop.desktop DESTINATION ${SERVICES_INSTALL_DIR})