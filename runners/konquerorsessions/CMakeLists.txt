set(krunner_konquerorsessions_SRCS
    konquerorsessions.cpp
)

kde4_add_plugin(krunner_konquerorsessions ${krunner_konquerorsessions_SRCS})
target_link_libraries(krunner_konquerorsessions ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS krunner_konquerorsessions DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES konquerorsessions.desktop DESTINATION ${SERVICES_INSTALL_DIR})

