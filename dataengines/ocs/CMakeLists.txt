
find_package(LibAttica REQUIRED)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${LIBATTICA_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/lib )
link_directories(${LIBATTICA_LIBRARY_DIRS})

set(ocs_engine_SRCS
   servicejobwrapper.cpp
   personservice.cpp
   ocsengine.cpp
)

kde4_add_plugin(plasma_engine_ocs ${ocs_engine_SRCS})
target_link_libraries(plasma_engine_ocs ${LIBATTICA_LIBRARIES} ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS}
  ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBS}  ${QT_QTNETWORK_LIBRARY} )

install(TARGETS plasma_engine_ocs DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-ocs.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES ocsPerson.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
