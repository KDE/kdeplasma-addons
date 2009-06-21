include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/lib )

add_subdirectory( lib )

set(ocs_engine_SRCS
   ocsengine.cpp
)


kde4_add_plugin(plasma_engine_ocs ${ocs_engine_SRCS})
target_link_libraries(plasma_engine_ocs ocsclient ${KDE4_KDECORE_LIBS}
  ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBS}  ${QT_QTNETWORK_LIBRARY} )

install(TARGETS plasma_engine_ocs DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-ocs.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
