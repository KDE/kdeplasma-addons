set(konqprofilesengine_SRCS
    konqprofilesengine.cpp
    konqprofilesservice.cpp
)

kde4_add_plugin(plasma_engine_konqprofiles ${konqprofilesengine_SRCS})
target_link_libraries(plasma_engine_konqprofiles ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_engine_konqprofiles DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-konqprofiles.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES org.kde.plasma.dataengine.konqprofiles.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

