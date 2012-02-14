set(konsoleprofilesengine_SRCS
    konsoleprofilesengine.cpp
    konsoleprofilesservice.cpp
)

kde4_add_plugin(plasma_engine_konsoleprofiles ${konsoleprofilesengine_SRCS})
target_link_libraries(plasma_engine_konsoleprofiles ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_engine_konsoleprofiles DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-konsoleprofiles.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install(FILES org.kde.plasma.dataengine.konsoleprofiles.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

