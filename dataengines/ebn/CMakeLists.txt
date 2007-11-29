set(ebn_engine_SRCS
    ebnengine.cpp
)

kde4_add_plugin(plasma_engine_ebn ${ebn_engine_SRCS})
target_link_libraries(plasma_engine_ebn ${KDE4_KDECORE_LIBS} ${KDE4_SYNDICATION_LIBS} ${PLASMA_LIBS})

install(TARGETS plasma_engine_ebn DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-ebn.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

