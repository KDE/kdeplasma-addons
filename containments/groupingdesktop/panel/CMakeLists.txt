
set(groupingpanel_SRCS groupingpanel.cpp)

kde4_add_plugin(plasma_containment_groupingpanel ${groupingpanel_SRCS})
target_link_libraries(plasma_containment_groupingpanel ${KDE4_PLASMA_LIBS} plasma_groupingcontainment)

install(TARGETS plasma_containment_groupingpanel DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-groupingpanel.desktop DESTINATION ${SERVICES_INSTALL_DIR})