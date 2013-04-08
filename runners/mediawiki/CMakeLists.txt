set(krunner_mediawiki_SRCS
    mediawikirunner.cpp
    mediawiki.cpp
)

kde4_add_plugin(krunner_mediawiki ${krunner_mediawiki_SRCS})
target_link_libraries(krunner_mediawiki ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS krunner_mediawiki DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES
        plasma-runner-wikipedia.desktop
        plasma-runner-wikitravel.desktop
        plasma-runner-techbase.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

# doesn't seem to work, maybe XML API is disabled
#install(FILES plasma-runner-userbase.desktop DESTINATION ${SERVICES_INSTALL_DIR})

