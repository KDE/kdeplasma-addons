set(krunner_mediawiki_SRCS
    mediawikirunner.cpp
    mediawiki.cpp
)

kde4_add_plugin(krunner_mediawiki ${krunner_mediawiki_SRCS})
target_link_libraries(krunner_mediawiki ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS krunner_mediawiki DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES wikipediarunner.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES wikitravelrunner.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES techbaserunner.desktop DESTINATION ${SERVICES_INSTALL_DIR})

# doesn't seem to work, maybe XML API is disabled
#install(FILES userbaserunner.desktop DESTINATION ${SERVICES_INSTALL_DIR})

